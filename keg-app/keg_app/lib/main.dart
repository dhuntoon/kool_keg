import 'dart:convert';
import 'package:keg_app/keg.dart';
import 'package:keg_app/keglist.dart';
import 'package:flutter/material.dart';
import 'package:keg_app/httploader.dart';

// Main control function, starts the program at launch
void main() {
  runApp(const MyApp());
}

// Function to generate static (hard-coded) list of kegs
// This function is called for testing purposes
// i.e., it will NOT be used in the final deployment
List<Keg> getStaticKegs() {
  List<Map<String, dynamic>> mapList = [];

  // decode the json into mappings
  Map<String, dynamic> map1 =
      // static json code for keg 1
      jsonDecode('{"data":[{"temp":"45","vol":"999","id":"1"}],"kegId":"1"}');
  Map<String, dynamic> map2 =
      // static json code for keg 2
      jsonDecode('{"data":[{"temp":"30","vol":"888","id":"1"}],"kegId":"2"}');

  // add the mappings to a map list
  mapList.add(map1);
  mapList.add(map2);

  // generate the list of kegs from the map list
  KegList kegList = KegList.fromMapList(mapList);
  List<Keg> kegs = kegList.getKegs();

  // modify the names of the static kegs
  kegs[0].setName("Static keg 1");
  kegs[1].setName("Static keg 2");

  return kegs;
}

List<Keg> getKegs() {
  KegList keglist = KegList();
  keglist.createDummyKeg();
  return keglist.getKegs();
}

// Widget used in body of MyApp
// inherits Stateful Widget object
class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key});

  @override
  State<MyHomePage> createState() => _MyHomePage();
}

// State object for KegList
class _MyHomePage extends State<MyHomePage> {
  // STATIC keg objects for testing
  // will be removed in final deployment
  List<Keg> kegs = getKegs();
  bool staticTest = false;

  Future<void> updateKegs() async {
    if (staticTest == true) return;
    List<Keg> updatedKegs = await getKegs();

    for (int i = 1; i < updatedKegs.length + 1; i++) {
      if (i > kegs.length) {
        kegs[0] = updatedKegs[i];
        continue;
      }

      DataEntry data = updatedKegs[i].getDataEntry();
      kegs[i].addDataEntry(data);
    }
  }

  // Nested widget for KegList; returns scaffold object
  @override
  Widget build(BuildContext context) {
    updateKegs();
    return Scaffold(
        // title bar at top of screen
        appBar:
            AppBar(title: const Text('Kool Keg - Mobile Monitoring System')),

        // list of objects containing cards
        body: ListView.builder(
          padding: const EdgeInsets.all(8.0),
          itemCount: kegs.length,
          itemBuilder: (context, i) {
            // card containing listTile with information from keg object
            return Card(
                child: ListTile(
                    title: Text(kegs[i].name),
                    subtitle: Text(
                        "ID: ${kegs[i].id}   T: ${kegs[i].getTemp()}   V: ${kegs[i].getVol()}")));
          },
        ));
  }
}

// MyApp object, uses Widget object to build app
class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // declare a nested widget object, which returns a MaterialApp
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: "Keg App",
      theme: ThemeData(
          appBarTheme: const AppBarTheme(
              backgroundColor: Colors.black, foregroundColor: Colors.white)),
      home: const MyHomePage(), // this Stateful Widget is declared above
    );
  }
}
