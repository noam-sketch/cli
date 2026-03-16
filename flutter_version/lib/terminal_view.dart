import 'dart:convert';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:xterm/xterm.dart';
import 'package:flutter_pty/flutter_pty.dart';

class CustomTerminalView extends StatefulWidget {
  final String? initialCwd;
  final Function(String) onSplitHorizontally;
  final Function(String) onSplitVertically;
  final VoidCallback onClose;

  const CustomTerminalView({
    super.key,
    this.initialCwd,
    required this.onSplitHorizontally,
    required this.onSplitVertically,
    required this.onClose,
  });

  @override
  State<CustomTerminalView> createState() => _CustomTerminalViewState();
}

class _CustomTerminalViewState extends State<CustomTerminalView> {
  late Terminal terminal;
  late Pty pty;
  final TerminalController _terminalController = TerminalController();
  
  @override
  void initState() {
    super.initState();
    
    terminal = Terminal(
      maxLines: 10000,
    );
    
    _startPty();
  }

  void _startPty() {
    String shell = Platform.environment['SHELL'] ?? 'bash';
    String? workingDirectory = widget.initialCwd ?? Platform.environment['HOME'];

    pty = Pty.start(
      shell,
      columns: terminal.viewWidth,
      rows: terminal.viewHeight,
      workingDirectory: workingDirectory,
      environment: Platform.environment,
    );

    pty.output.cast<List<int>>().transform(const Utf8Decoder(allowMalformed: true)).listen((text) {
      terminal.write(text);
    });

    terminal.onOutput = (text) {
      pty.write(const Utf8Encoder().convert(text));
    };

    terminal.onResize = (width, height, pixelWidth, pixelHeight) {
      pty.resize(height, width);
    };
    
    pty.exitCode.then((code) {
      widget.onClose();
    });
  }

  String _getCurrentWorkingDirectory() {
    try {
      // Find the current working directory of the shell process
      final link = File('/proc/${pty.pid}/cwd').resolveSymbolicLinksSync();
      return link;
    } catch (e) {
      return Platform.environment['HOME'] ?? '/';
    }
  }

  void _copy() {
    final selection = _terminalController.selection;
    if (selection != null) {
      final text = terminal.buffer.getText(selection);
      if (text.isNotEmpty) {
        Clipboard.setData(ClipboardData(text: text));
      }
    }
  }

  void _paste() async {
    final data = await Clipboard.getData('text/plain');
    if (data != null && data.text != null) {
      terminal.paste(data.text!);
    }
  }

  void _showContextMenu(BuildContext context, Offset position) async {
    final String cwd = _getCurrentWorkingDirectory();
    
    final RenderBox overlay = Overlay.of(context).context.findRenderObject() as RenderBox;
    final result = await showMenu<String>(
      context: context,
      position: RelativeRect.fromRect(
        Rect.fromPoints(position, position),
        Offset.zero & overlay.size,
      ),
      items: <PopupMenuEntry<String>>[
        const PopupMenuItem<String>(value: 'copy', child: Text('Copy')),
        const PopupMenuItem<String>(value: 'paste', child: Text('Paste')),
        const PopupMenuDivider(),
        const PopupMenuItem<String>(value: 'split_h', child: Text('Split Horizontally')),
        const PopupMenuItem<String>(value: 'split_v', child: Text('Split Vertically')),
        const PopupMenuDivider(),
        const PopupMenuItem<String>(value: 'close', child: Text('Close Terminal Panel')),
      ],
    );

    switch (result) {
      case 'copy':
        _copy();
        break;
      case 'paste':
        _paste();
        break;
      case 'split_h':
        widget.onSplitHorizontally(cwd);
        break;
      case 'split_v':
        widget.onSplitVertically(cwd);
        break;
      case 'close':
        pty.kill();
        widget.onClose();
        break;
    }
  }

  @override
  void dispose() {
    pty.kill();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onSecondaryTapDown: (details) {
        _showContextMenu(context, details.globalPosition);
      },
      child: TerminalView(
        terminal,
        controller: _terminalController,
        autofocus: true,
        textStyle: const TerminalStyle(
          fontFamily: 'UbuntuMono',
          fontSize: 14,
        ),
      ),
    );
  }
}
