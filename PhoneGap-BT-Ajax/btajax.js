function ResponseParser() {

	this.states = {
		PARSING_HEADERS: 0,
		PARSING_BODY: 1,
		PARSING_RESPONSE_STATUS: 2
	};

	this.responseStatusExpression = /HTTP\/(\d)\.(\d) (\d{3}) (.*)/i;

}

ResponseParser.prototype.parse = function (data, callback) {

	var res = {
		body: "",
		statusCode: 0,
		statusText: "",
		httpVersion: 0,
		headers: []
	};
	var current_line = [];

	var state = this.states.PARSING_RESPONSE_STATUS;

	for (var i = 0; i < data.length; i++) {

		var c = data.charAt(i);

		if (state == this.states.PARSING_BODY) {
			try {
				res.body = JSON.parse(data.substring(i, data.length));
			}
			catch (e) {
				res.body = data.substring(i, data.length);
			}
			break;
		}
		if (c == '\n') {
			if (current_line.length == 0) {
				state = this.states.PARSING_BODY;
			} else {
				var line = current_line.join("");
				if (state == this.states.PARSING_RESPONSE_STATUS) {
					var match = this.responseStatusExpression.exec(line);
					if (match == null) {
						return callback(new Error("Failed to parse response"), null);
					}
					res.statusCode = match[3];
				}
				else {
					var lastIndex = line.indexOf(":");
					res.headers.push(
						{
							name: line.substring(0, lastIndex),
							value: line.substring(lastIndex + 1).trim()
						}
					);
				}
				state = this.states.PARSING_HEADERS;
				current_line = [];
			}
			continue;
		} else if (c == '\r') {
			continue;
		}

		current_line.push(c);
	}

	return callback(null, res);
};


(function ($) {

	var oldAjax = $.ajax;

	$.ajax = function (options) {

		var deferred = $.Deferred();

		if ($.ajax.useBluetooth === false) {
			return oldAjax.apply(this, arguments);
		}

		bluetoothSerial.isConnected(
			function () {
				makeRequest();
			},
			function () {
				connectBT(function (err) {
					if (!err) {

						makeRequest();
					}
					else {
						console.log(err);
					}

				});
			}
		);

		function connectBT (cb) {

			bluetoothSerial.list(
				function (results) {

					var deviceList = [];

					for (var i = 0; i < results.length; i++) {
						var deviceListItem = {
							classNumber: results[i].class,
							name: results[i].name,
							address: results[i].address
						};

						deviceList.push(deviceListItem);
					}

					var connectPopupTemplate =
						'<div id="bt-ajax-popup">' +
							'<div class="container">' +
							'<div class="form-group">' +
							'<label for="select-bt-device"><h3>Connect to device</h3></label>' +
							'<select id = "select-bt-device"  class="form-control">' +
							'<option selected disabled>Choose one...</option>' +
							'</select>' +
							'</div>' +
							'<a class="btn btn-primary" id="close-bt-ajax-popup">Close<a>' +
							'</div>' +
							'</div>';


					var connectPopup = $(connectPopupTemplate);

					var options = connectPopup.find('#select-bt-device');

					for (i = 0; i < deviceList.length; i++) {
						var option = $('<option value="' + deviceList[i].address + '">' + deviceList[i].name + '</option>');
						options.append(option);
					}

					options.on('change', function () {
						bluetoothSerial.connect(this.value, function () {
							$("#bt-ajax-popup").remove();
							cb();

						}, function () {
							cb(new Error('Connection failed'));
						});
					});

					connectPopup.find('#close-bt-ajax-popup').on('click', function () {
						$("#bt-ajax-popup").remove();
						cb(new Error('User cancelled'));
					});

					connectPopup.css("width", $(window).width());
					connectPopup.css("height", $(window).height());
					connectPopup.css("position", "absolute");
					connectPopup.css("top", 0);
					connectPopup.css("left", 0);
					connectPopup.css("left", 0);
					connectPopup.css("z-index", 10000000000);
					connectPopup.css("background", "#fff");

					$("body").append(connectPopup);


				},
				function (error) {
					console.log(error);
				}
			);

		}


		function makeRequest() {
			var request = "";

			options.url = (options.url[0] == '/') ? options.url.substr(1) : options.url;


			switch (options.type) {
				case 'POST':
					request += "POST /" + options.url + " HTTP 1.0\r\n";
					request += "Content-Length: " + options.data.length + "\r\n\r\n";
					request += options.data;
					break;

				case 'PUT':
					request += "PUT /" + options.url + " HTTP 1.0\r\n";
					request += "Content-Length: " + options.data.length + "\r\n\r\n";
					request += options.data;
					break;

				case 'DELETE':
					request += "DELETE /" + options.url + " HTTP 1.0 \r\n\r\n";
					break;

				case 'GET':
					request += "GET /" + options.url + " HTTP 1.0 \r\n\r\n";
					break;
			}

			console.log(request);

			bluetoothSerial.clear(function () {
				bluetoothSerial.write(request, function () {

					checkIfMore(0);

					function checkIfMore(last) {
						setTimeout(function () {
							bluetoothSerial.available(function (numBytes) {
								if (last != 0 && last == numBytes) {
									bluetoothSerial.read(function (data) {
										console.log(data);
										var parser = new ResponseParser();
										parser.parse(data, function (err, res) {
											if (!err) {
												if (res.statusCode >= 200 && res.statusCode < 300) {
													if (options.success) {
														options.success(res.body);
													}
													if (options.complete) {
														options.complete(res.body);
													}
													deferred.resolve(res.body);
												}
												else {
													return deferred.reject(res.statusCode);
												}
											}
											else {
												return deferred.reject(err);
											}
										})

									});
								}
								else {
									checkIfMore(numBytes);
								}
							});
						}, 100);
					}
				});
			});
			return deferred.promise();
		}
	};

})(jQuery);