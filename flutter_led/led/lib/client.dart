import 'dart:async';
import 'package:http/http.dart' as http;

class Client {

  Map<String, DateTime> requestTimeMap = {};
  String ip;

  Client(this.ip);

  
  void sendSpeed(int value) async 
  {
    String request = 'set-speed';
    if (!isRequestAllowed(request)) return;

    print("sendColor $value");
      var response = await fetchRequest("$request?speed=$value");
    print(response.statusCode);
  }

  void sendBrightness(int value) async 
  {
    String request = 'set-brightness';
    if (!isRequestAllowed(request)) return;

    print("sendBrightness $value");
    var response = await fetchRequest("$request?brightness=$value");
    print(response.statusCode);
  }

  bool isRequestAllowed(String request)
  {
    DateTime? lastReuqestTime = requestTimeMap[request];
    var now = DateTime.now();
    if (lastReuqestTime != null
        && now.difference(lastReuqestTime).inMilliseconds < 100)
    {
      return false;
    }

    requestTimeMap[request] = now;
    return true;
  }

  Future<int> fetchColor() async
  {
    var response = await fetchRequest("get-color");
    print("response statusCode: ${response.statusCode}");

    if (response.statusCode == 200)
    {
      int color = int.parse(response.body); 
      print("fetchColor response: ${color}");
      return color;
    }

    return 0;
  }

  void sendColor(int hue) async
  {
    String request = 'set-color';
    if (!isRequestAllowed(request)) return;

    print("sendColor $hue");
    var response = await fetchRequest("$request?hue=$hue");
    print(response.statusCode);
  }

  Future<http.Response> fetchRequest(String request) async {

    var url = getUrl(request);
    print("fetchRequese url=$url");

    http.Response response;

    try{
      response = await http.get(Uri.parse(url));
    }
    catch (e) {
      print("error");
    }

    return http.Response("", 400);
  }

  String getUrl(String request)
  {
    return "http://$ip/$request";
  }

  void sendBeat() async {
    print("beat");
    var response = await fetchRequest("beat");
    print(response.statusCode);
  }
}