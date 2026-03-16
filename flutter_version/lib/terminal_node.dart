import 'package:flutter/material.dart';

class TerminalNode {
  final String id;
  TerminalNode? parent;
  List<TerminalNode> children;
  Axis direction;
  Widget? terminalView;
  String? initialCwd;

  TerminalNode({
    required this.id,
    this.parent,
    this.children = const [],
    this.direction = Axis.horizontal,
    this.terminalView,
    this.initialCwd,
  });

  bool get isLeaf => children.isEmpty;
}
