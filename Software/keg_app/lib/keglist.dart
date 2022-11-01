import 'package:keg_app/keg.dart';
import 'package:keg_app/httploader.dart';
import 'dart:convert';

class KegList {
  KegList();
  List<Keg> kegs = [];

  // build a list of kegs given a list of json maps
  // NOTE: expects parameter data to have been DECODED prior to entry
  factory KegList.fromMapList(List<Map<String, dynamic>> data) {
    KegList kegList = KegList();
    for (int i = 0; i < data.length; i++) {
      Keg k = Keg.fromJson(data[i]);
      kegList.addKeg(k);
    }
    return kegList;
  }

  void addKeg(Keg keg) {
    kegs.add(keg);
  }

  List<Keg> getKegs() {
    return kegs;
  }

  void pullFromHTTP() async {
    try {
      for (int i = 1; i < 20; i++) {
        HTTPLoader loader = HTTPLoader(i);
        String jsonKeg = await loader.getKegData();
        jsonKeg = jsonKeg.substring(1, jsonKeg.length - 1);
        Keg k = Keg.fromJson(jsonDecode(jsonKeg));
        addKeg(k);
      }
    } catch (err) {
      print("Error Code: " + err.toString());
    }
  }
}
