import 'package:flutter/material.dart';

void main() {
  runApp(const MyApp());
}

class Keg {
  int temp = 0;
  int vol = 0;
  bool connection = false;
  String ID;
  String name = "unnamed";

  Keg(this.ID);
  Keg.testing(this.ID, this.name, this.connection, this.temp, this.vol);

  @override
  String toString() {
    return "Keg [ID=$ID, name=$name, temp=$temp, vol=$vol]";
  }
}

class KegList extends StatefulWidget {
  const KegList({super.key});

  @override
  State<KegList> createState() => _KegListState();
}

class _KegListState extends State<KegList> {
  List<Keg> kegs = [
    Keg.testing("237423984", "first beverage name goes here", true, 12, 34),
    Keg.testing("847585473", "the 2nd beverage is here", true, 31, 12),
    Keg("123456789")
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar:
            AppBar(title: const Text('Kool Keg - Mobile Monitoring System')),
        body: ListView.builder(
          padding: const EdgeInsets.all(8.0),
          itemCount: kegs.length,
          itemBuilder: (context, i) {
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

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: "Keg App",
      theme: ThemeData(
          appBarTheme: const AppBarTheme(
              backgroundColor: Colors.black, foregroundColor: Colors.white)),
      home: const KegList(),
    );
  }
}
