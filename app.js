Pebble.addEventListener("appmessage", function(e) {
sendToServer(); }
);
function sendToServer() {
var req = new XMLHttpRequest();
var ipAddress = "165.123.221.116"; // Hard coded IP address
var port = "3001"; // Same port specified as argument to server 
var url = "http://" + ipAddress + ":" + port + "/";
var method = "GET";
var async = true;
req.onload = function(e) {
// see what came back
var msg = "no response";
var response = JSON.parse(req.responseText); 
if (response) {
if (response.name) {
msg = response.name;
}
else msg = "noname"; 
}
// sends message back to pebble 
Pebble.sendAppMessage({ "0": msg });
};
req.open(method, url, async); 
req.send(null);
}