<h3 id='router-Router'>Router::Router</h3>
<h4 class='variant'>Router(const char* urlPrefix = "");</h4>

Router constructor takes a parameter the prefix that will be used for routering requests. For example e router defined as Router router("/api") will try to find matching middleware when the request url begins with “/api”. Te prefix can be left empty to make the router middleware handlers mounted to the root path.
