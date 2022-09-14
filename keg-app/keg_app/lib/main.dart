import 'package:flutter/material.dart';

void main() {
  runApp(const MyApp());
}

class KegList extends StatefulWidget {
  const KegList({super.key});

  @override
  State<KegList> createState() => _KegListState();
}

class _KegListState extends State<KegList> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar:
            AppBar(title: const Text('Kool Keg - Mobile Monitoring System')),
        body: ListView.builder(
          padding: const EdgeInsets.all(8.0),
          itemBuilder: (context, i) {
            if (i.isOdd) return const Divider();

            final index = i ~/ 2;
            return ListTile(
              title: Text("Beverage Name for Keg $index"),
              subtitle: const Text("ID: 123456789"),
              trailing: const Icon(
                Icons.check_circle,
                color: Colors.green,
                semanticLabel: 'Edit',
              ),
            );
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
              backgroundColor: Colors.blue, foregroundColor: Colors.white)),
      home: const KegList(),
    );
  }
}
