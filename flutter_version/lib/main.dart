import 'package:flutter/material.dart';
import 'package:window_manager/window_manager.dart';
import 'workspace.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await windowManager.ensureInitialized();

  WindowOptions windowOptions = const WindowOptions(
    size: Size(1024, 768),
    center: true,
    backgroundColor: Colors.transparent,
    skipTaskbar: false,
    titleBarStyle: TitleBarStyle.hidden,
  );
  
  windowManager.waitUntilReadyToShow(windowOptions, () async {
    await windowManager.show();
    await windowManager.focus();
  });

  runApp(const CliApp());
}

class CliApp extends StatelessWidget {
  const CliApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Cli',
      theme: ThemeData(
        brightness: Brightness.dark,
        scaffoldBackgroundColor: const Color(0xFF1E1E1E),
        fontFamily: 'UbuntuMono',
      ),
      home: const MainScreen(),
      debugShowCheckedModeBanner: false,
    );
  }
}

class MainScreen extends StatelessWidget {
  const MainScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Column(
        children: [
          // Custom Window Title Bar
          DragToMoveArea(
            child: Container(
              height: 32,
              color: const Color(0xFF2D2D2D),
              child: Row(
                children: [
                  const SizedBox(width: 10),
                  Image.asset('assets/icon.png', width: 16, height: 16, errorBuilder: (context, error, stackTrace) => const Icon(Icons.terminal, size: 16, color: Colors.green)),
                  const SizedBox(width: 8),
                  const Text('Cli', style: TextStyle(color: Colors.white, fontSize: 14)),
                  const Spacer(),
                  // Window controls
                  IconButton(
                    icon: const Icon(Icons.remove, size: 16, color: Colors.white70),
                    onPressed: () => windowManager.minimize(),
                    padding: EdgeInsets.zero,
                    constraints: const BoxConstraints(),
                    hoverColor: Colors.white24,
                  ),
                  const SizedBox(width: 12),
                  IconButton(
                    icon: const Icon(Icons.crop_square, size: 16, color: Colors.white70),
                    onPressed: () async {
                      if (await windowManager.isMaximized()) {
                        windowManager.unmaximize();
                      } else {
                        windowManager.maximize();
                      }
                    },
                    padding: EdgeInsets.zero,
                    constraints: const BoxConstraints(),
                    hoverColor: Colors.white24,
                  ),
                  const SizedBox(width: 12),
                  IconButton(
                    icon: const Icon(Icons.close, size: 16, color: Colors.white70),
                    onPressed: () => windowManager.close(),
                    padding: EdgeInsets.zero,
                    constraints: const BoxConstraints(),
                    hoverColor: Colors.red.withOpacity(0.8),
                  ),
                  const SizedBox(width: 10),
                ],
              ),
            ),
          ),
          // Main Workspace
          const Expanded(
            child: Workspace(),
          ),
        ],
      ),
    );
  }
}
