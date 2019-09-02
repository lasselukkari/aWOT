<h3 id='req.client'>Stream * client();</h3>

Returns a pointer to the raw tcp client. You should not normally need to use this. You need to know what you are doing. Reading from the client will not add to the the result of bytesRead() and you need to take care of request timeouts.
