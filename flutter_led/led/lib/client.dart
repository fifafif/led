import 'package:http/http.dart' as http;

class Client{
  
  String ip;

  Client(this.ip){
  }

  Future<int> fetchColor() async
  {
    var response = await fetchRequest("get-color");
    print("response $response");

    if (response.statusCode == 200)
    {
      return int.parse(response.body);
    }

    return 0;
  }

  void sendColor(int hue) async
  {
    print("sendColor $hue");
    var response = await fetchRequest("set-color/$hue");
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
}