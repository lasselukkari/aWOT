#pragma once

#include "OstreamHelpers.h"
#include "Assertion.h"
#include <iostream>
using namespace std;

struct Results {
  int passed;
  int failed;
  int skipped;
};

struct TestData {
  const char* name;
  int result;
};

class TestSetup
{
  public:
  TestSetup() {
    sInstance = this;
  }
  virtual void run() {}
  static TestSetup *sInstance;
};

class TestTeardown
{
  public:
  TestTeardown() {
    sInstance = this;
  }
  virtual void run() {}
  static TestTeardown *sInstance;
};

class Test
{
  public:
    class ReporterTAP {
      private:
        int mTestCounter;
        int mAssertCounter;

      public:
        ReporterTAP() {}
        ~ReporterTAP() {}

        void onTestRunInit(int numTests) {
          cerr << "TAP version 13" << endl;
          cerr << 1 << ".." << numTests << endl;
          mTestCounter = 0;
        }

        void onTestStart(TestData td) {
          mAssertCounter = 0;
          ++mTestCounter;
          cerr << "# Subtest: " << td.name << endl;
        }

        void onTestEnd(TestData td) {
          cerr << "    1.." << mAssertCounter << endl;
          if (td.result == RESULT_PASS) {
            cerr << "ok " << mTestCounter << " - " << td.name << endl;
          } else {
            cerr << "not ok " << mTestCounter << " - " << td.name << endl;
          }
        }

        void onAssert(
          const char* file,
          int line,
          const char* description,
          bool pass
        ) {
          cerr << "    " << (pass ? "" : "not ") << "ok " << ++mAssertCounter << " - " << description << endl;
          if (!pass) {
            cerr << "      ---" << endl;
            cerr << "      at:" << endl;
            cerr << "        file: " << file << endl;
            cerr << "        line: " << line << endl;
            cerr << "      ..." << endl;
          }
        }

        template <typename A, typename B> void onAssert(
          const char* file,
          int line,
          const char* description,
          bool pass,
          const char* lhsRelevance,
          const char* lhsLabel,
          const A &lhs,
          const char* opLabel,
          const char* rhsRelevance,
          const char* rhsLabel,
          const B &rhs
        ) {
          cerr << "    " << (pass ? "" : "not ") << "ok " << ++mAssertCounter << " - ";
          cerr << description << " " << lhsLabel << " " << opLabel << " " << rhsLabel << endl;
          if (!pass) {
            cerr << "      ---" << endl;
            cerr << "      operator: " << opLabel << endl;
            cerr << "      " << lhsRelevance << ": " << lhs << endl;
            cerr << "      " << rhsRelevance << ": " << rhs << endl;
            cerr << "      at:" << endl;
            cerr << "        file: " << file << endl;
            cerr << "        line: " << line << endl;
            cerr << "      ..." << endl;
          }
        }
    };

  private:
    ReporterTAP* mReporter;
    const char* mName;

    static Test* sRoot;
    Test* mNext;

    void append() {
      if (!sRoot) return (void)(sRoot = this);
      Test* p;
      for (p = sRoot; p->mNext; p = p->mNext);
      p->mNext = this;
    }

    void excise() {
      for (Test **p = &sRoot; *p != 0; p=&((*p)->mNext)) {
        if (*p == this) return (void)(*p = (*p)->mNext);
       }
    }

    static int numTests() {
      if (!sRoot) return 0;
      int i = 1;
      for (Test* p = sRoot; p->mNext; ++i && (p = p->mNext));
      return i;
    }

    int mResult;

  public:
    static const int RESULT_NONE = 0;
    static const int RESULT_PASS = 1;
    static const int RESULT_FAIL = 2;
    static const int RESULT_SKIP = 3;

    inline const char *name() const { return mName; }
    inline int result() const { return mResult; }

    Test(const char* _name) : mName(_name) {
      mResult = RESULT_NONE;
      mReporter = 0;
      append();
    }

    inline void fail() { mResult = RESULT_FAIL; }
    inline void skip() { mResult = RESULT_SKIP; }

    static Results run(ReporterTAP* reporter) {
      if (reporter) reporter->onTestRunInit(numTests());
      Results results = {0, 0, 0};

      for (Test *p = sRoot; p; p = p->mNext) {
        p->prepare();
        p->mReporter = reporter;
        TestData t1 = {p->name(), p->result()};
        if (reporter) reporter->onTestStart(t1);
        if (TestSetup::sInstance) TestSetup::sInstance->run();
        p->test();
        if (TestTeardown::sInstance) TestTeardown::sInstance->run();
        if (p->mResult == RESULT_PASS) ++results.passed;
        if (p->mResult == RESULT_FAIL) ++results.failed;
        if (p->mResult == RESULT_SKIP) ++results.skipped;
        TestData t2 = {p->name(), p->result()};
        if (reporter) reporter->onTestEnd(t2);
      }

      return results;
    }

    static int run_and_report(int argc, char *argv[]) {
      ReporterTAP rep;
      Results results = run(&rep);
      return results.failed + results.skipped;
    }

    void prepare() {
      mResult = RESULT_PASS;
    }

    void test() {
      task();
    }

    virtual void task() = 0;

    virtual ~Test() {
      excise();
    }

    bool assertion(
        const char *file,
        int line,
        const char *description,
        bool ok)
    {
      if (mReporter) {
        mReporter->onAssert(file, line, description, ok);
      }

      if (!ok)
        fail();
      return ok;
    }

    template <typename A, typename B>
    bool assertion(
        const char *file,
        int line,
        const char *description,
        const char *lhsRelevance,
        const char *lhsLabel,
        const A &lhs,

        const char *ops,

        bool (*op)(
            const A &lhs,
            const B &rhs),

        const char *rhsRelevance,
        const char *rhsLabel,
        const B &rhs)
    {
      bool ok = op(lhs, rhs);

      if (mReporter) {
        mReporter->onAssert(file, line, description, ok,
          lhsRelevance, lhsLabel, lhs, ops, rhsRelevance, rhsLabel, rhs);
      }

      if (!ok)
        fail();
      return ok;
    }

};

#define unittest(name)             \
  struct test_##name : Test        \
  {                                \
    test_##name() : Test(#name){}; \
    void task();                   \
  } test_##name##_instance;        \
  void test_##name ::task()

#define unittest_setup()                          \
  class unittest_setup_class : public TestSetup { \
    public:                                       \
    virtual void run() override;                  \
  } unittest_setup_instance;                      \
  void unittest_setup_class::run()

#define unittest_teardown()                             \
  class unittest_teardown_class : public TestTeardown { \
    public:                                             \
    virtual void run() override;                        \
  } unittest_teardown_instance;                         \
  void unittest_teardown_class::run()

#define unittest_main()                      \
  int main(int argc, char *argv[]) {         \
    return Test::run_and_report(argc, argv); \
  }
