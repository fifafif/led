import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:led/client.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'LED',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // TRY THIS: Try running your application with "flutter run". You'll see
        // the application has a purple toolbar. Then, without quitting the app,
        // try changing the seedColor in the colorScheme below to Colors.green
        // and then invoke "hot reload" (save your changes or press the "hot
        // reload" button in a Flutter-supported IDE, or press "r" if you used
        // the command line to start the app).
        //
        // Notice that the counter didn't reset back to zero; the application
        // state is not lost during the reload. To reset the state, use hot
        // restart instead.
        //
        // This works for code too, not just values: Most code changes can be
        // tested with just a hot reload.
        colorScheme: ColorScheme.fromSeed(seedColor: const Color.fromARGB(255, 0, 190, 6)),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'LED'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {

  int _counter = 0;
  double _hueValue = 0;
  double _brightnessValue = 255;
  double _speedValue = 127;
  int _hueColor = 0;

  Client client = Client("192.168.4.1");  


    Color rgbFromWheel(int WheelPos)
    {
        int redValue, greenValue, blueValue;

        WheelPos = 255 - WheelPos;
        if (WheelPos < 85)
        {
            redValue = 255 - WheelPos * 3;
            greenValue = 0;
            blueValue = WheelPos * 3;
        }
        else if (WheelPos < 170)
        {
            WheelPos -= 85;
            redValue = 0;
            greenValue = WheelPos * 3;
            blueValue = 255 - WheelPos * 3;
        }
        else
        {
            WheelPos -= 170;
            redValue = WheelPos * 3;
            greenValue = 255 - WheelPos;
            blueValue = 0;
        }

        return new Color(0xFF << 24 | redValue << 16 | greenValue << 8 | blueValue);
    }



  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        // TRY THIS: Try changing the color here to a specific color (to
        // Colors.amber, perhaps?) and trigger a hot reload to see the AppBar
        // change color while the other colors stay the same.
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        // Here we take the value from the MyHomePage object that was created by
        // the App.build method, and use it to set our appbar title.
        title: Text(widget.title),
      ),
      body: Center(
        // Center is a layout widget. It takes a single child and positions it
        // in the middle of the parent.
        child: Column(
          // Column is also a layout widget. It takes a list of children and
          // arranges them vertically. By default, it sizes itself to fit its
          // children horizontally, and tries to be as tall as its parent.
          //
          // Column has various properties to control how it sizes itself and
          // how it positions its children. Here we use mainAxisAlignment to
          // center the children vertically; the main axis here is the vertical
          // axis because Columns are vertical (the cross axis would be
          // horizontal).
          //
          // TRY THIS: Invoke "debug painting" (choose the "Toggle Debug Paint"
          // action in the IDE, or press "p" in the console), to see the
          // wireframe for each widget.
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Image(
              image: AssetImage('assets/t_hue_colors.png'),
              width: double.infinity,
              fit: BoxFit.cover,
            ),
            Slider(
              min: 0,
              max: 255,
              value: _hueValue,
              activeColor: rgbFromWheel(_hueValue.toInt()),
              onChanged: (value) {
                setState(() {
                  _hueValue = value;
                  client.sendColor(_hueValue.toInt());
                });
              },
            ),
            ElevatedButton(
              child: const Text('Get Color'),
              onPressed: () {
                print("get color");
                client.fetchColor().then((value) {
                  setState(() {
                    _hueValue = value.toDouble();
                  });
                });
              },
            ),
            const Text(
              'Speed:',
            ),
            Slider(
              min: 0,
              max: 255,
              value: _speedValue,
              onChanged: (value) {
                setState(() {
                  _speedValue = value;
                  client.sendSpeed(_speedValue.toInt());
                });
              },
            ),
            const Text(
              'Brightness:',
            ),
            Slider(
              min: 0,
              max: 255,
              value: _brightnessValue,
              onChanged: (value) {
                setState(() {
                  _brightnessValue = value;
                  client.sendBrightness(_brightnessValue.toInt());
                });
              },
            ),
          ],
        ),
      ),
      /*floatingActionButton: FloatingActionButton(
        onPressed: _incrementCounter,
        tooltip: 'Increment',
        child: const Icon(Icons.add),
      ), // This trailing comma makes auto-formatting nicer for build methods.*/
    );
  }
}
