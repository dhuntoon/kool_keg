import 'package:http/http.dart' as http;

// https://1cl4lag6ba.execute-api.us-east-2.amazonaws.com/prod/keglist?kegId=1
class HTTPLoader {
  HTTPLoader(this.id);
  final int id;

  Future<String> httpGET(String server, String path) async {
    var url = Uri.https(server, path, {'q': '{http}', 'kegId': id.toString()});
    var response = await http.get(url);
    if (response.statusCode == 200) {
      print("Retrieved from AWS: " + response.body);
      return response.body;
    } else {
      print('Request failed with status: ${response.statusCode}.');
      return "Could not load";
    }
  }

  Future<String> getKegData() {
    Future<String> pos = httpGET(
        '1cl4lag6ba.execute-api.us-east-2.amazonaws.com', '/prod/keglist');
    return pos;
  }
}
