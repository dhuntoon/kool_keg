import 'package:keg_app/httploader.dart';

// Keg object: stores device id and relevant monitoring data
class Keg {
  // constructor
  Keg({required this.id, required this.data});

  // object parameters
  final String id;
  List<DataEntry> data;
  String name = "unnamed";

  // constructor, adapted to build from json
  factory Keg.fromJson(Map<String, dynamic> input) {
    final id = input['kegId'] as String;
    final dataSet = input['data'] as List<dynamic>?;
    // build list of nested DataEntry objects
    final data = dataSet != null
        ? dataSet.map((dataEntry) => DataEntry.fromJson(dataEntry)).toList()
        : <DataEntry>[];

    return Keg(id: id, data: data);
  }

  // getters
  String getID() {
    return id;
  }

  String getTemp() {
    return data.last.getTemp();
  }

  String getVol() {
    return data.last.getVol();
  }

  DataEntry getDataEntry() {
    return data.last;
  }

  List<DataEntry> getDataList() {
    return data;
  }

  String getName() {
    return name;
  }

  // setters
  void setName(String name) {
    this.name = name;
  }

  void addDataEntry(DataEntry dataEntry) {
    data.add(dataEntry);
  }
}

// DataEntry object: stores a single record of temperature and volume
class DataEntry {
  // constructor
  DataEntry({required this.id, required this.temp, required this.vol});

  // object parameters
  final String id;
  final String temp;
  final String vol;

  // constructor, adapted to build from json
  factory DataEntry.fromJson(Map<String, dynamic> data) {
    final id = data['id'] as String;
    final temp = data['temp'] as String;
    final vol = data['vol'] as String;
    return DataEntry(id: id, temp: temp, vol: vol);
  }

  // getters
  String getID() {
    return id;
  }

  String getTemp() {
    return temp;
  }

  String getVol() {
    return vol;
  }
}
