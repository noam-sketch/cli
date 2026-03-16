import 'dart:convert';
import 'package:bidi/bidi.dart' as bidi;

void main() {
  String textWithAnsi = '\x1b[31mשלום\x1b[0m';
  print('Original: ' + textWithAnsi);
  print('Bidi 2: ' + bidi.logicalToVisual2(textWithAnsi));
}
