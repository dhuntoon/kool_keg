import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:keg_app/httploader.dart';

// Main control function, starts the program at launch
void main() {
  foo();
  runApp(const MyApp());
}

void foo() async {
  http_loader loader = http_loader();
  String data = await loader.getKegData();
}

// keg object: stores relevant data for each keg
class Keg {
  // Keg Parameters
  int temp = 0; // current temperature; initialized to zero
  List<int> tempData = [];
  int vol = 0; // remaining volume; initialized to zero
  List<int> volData = [];
  bool connection = false; // keg connection status to cloud service
  String ID; // keg ID (not modified)
  String name = "unnamed"; // customizable name; initialized to unnamed

  // primary constructor; used when a new keg is detected
  Keg(this.ID);

  // helper constructor; useful for updating keg while testing code
  // will be removed in final deployment
  Keg.testing(this.ID, this.name, this.connection, this.temp, this.vol);

  void updateData(int temp, int vol) {
    tempData.add(temp);
    this.temp = temp;
    this.vol = vol;
  }

  // helper function; useful for displaying keg data
  @override
  String toString() {
    return "Keg [ID=$ID, name=$name, temp=$temp, vol=$vol]";
  }
}

// Widget used in body of MyApp
// inherits Stateful Widget object
class KegList extends StatefulWidget {
  const KegList({super.key});

  @override
  State<KegList> createState() => _KegListState();
}

// State object for KegList
class _KegListState extends State<KegList> {
  // keg objects for testing
  // will be removed in final deployment
  List<Keg> kegs = [
    Keg.testing("237423984", "first beverage name goes here", true, 12, 34),
    Keg.testing("847585473", "the 2nd beverage is here", true, 31, 12),
    Keg("123456789")
  ];

  // Nested widget for KegList; returns scaffold object
  @override
  Widget build(BuildContext context) {
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
              subtitle: Text("ID: ${kegs[i].ID}"),
              trailing: Icon(
                kegs[i].connection == true ? Icons.check_circle : Icons.error,
                color:
                    kegs[i].connection == true ? Colors.green : Colors.orange,
                semanticLabel:
                    kegs[i].connection == true ? "Connected" : "Disconnected",
              ),
            ));
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
      home: const KegList(), // this Stateful Widget is declared above
    );
  }
}
