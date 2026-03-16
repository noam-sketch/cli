import 'package:flutter/material.dart';
import 'package:multi_split_view/multi_split_view.dart';
import 'terminal_node.dart';
import 'terminal_view.dart';

class Workspace extends StatefulWidget {
  const Workspace({super.key});

  @override
  State<Workspace> createState() => _WorkspaceState();
}

class _WorkspaceState extends State<Workspace> {
  late TerminalNode rootNode;

  @override
  void initState() {
    super.initState();
    rootNode = TerminalNode(id: 'root');
  }

  void _splitNode(TerminalNode targetNode, Axis direction, String cwd) {
    setState(() {
      TerminalNode newNode = TerminalNode(id: DateTime.now().millisecondsSinceEpoch.toString(), initialCwd: cwd);
      
      if (targetNode.parent == null) {
        // If it's the root node without a parent
        if (targetNode.children.isEmpty) {
          // It's a single terminal
          TerminalNode oldTerminal = TerminalNode(id: '${targetNode.id}_1', terminalView: targetNode.terminalView);
          targetNode.terminalView = null;
          targetNode.direction = direction;
          targetNode.children = [oldTerminal, newNode];
          oldTerminal.parent = targetNode;
          newNode.parent = targetNode;
        } else {
          // Root already has children
          if (targetNode.direction == direction) {
            targetNode.children.add(newNode);
            newNode.parent = targetNode;
          } else {
            // Need to wrap
            TerminalNode newParent = TerminalNode(id: '${targetNode.id}_wrapper');
            newParent.direction = targetNode.direction;
            newParent.children = List.from(targetNode.children);
            for (var child in newParent.children) {
              child.parent = newParent;
            }
            targetNode.children = [newParent, newNode];
            targetNode.direction = direction;
            newParent.parent = targetNode;
            newNode.parent = targetNode;
          }
        }
      } else {
        TerminalNode parent = targetNode.parent!;
        if (parent.direction == direction) {
          int index = parent.children.indexOf(targetNode);
          parent.children.insert(index + 1, newNode);
          newNode.parent = parent;
        } else {
          int index = parent.children.indexOf(targetNode);
          TerminalNode newParent = TerminalNode(id: '${targetNode.id}_wrapper');
          newParent.direction = direction;
          newParent.children = [targetNode, newNode];
          targetNode.parent = newParent;
          newNode.parent = newParent;
          parent.children[index] = newParent;
          newParent.parent = parent;
        }
      }
    });
  }

  void _closeNode(TerminalNode node) {
    setState(() {
      if (node.parent == null) {
        // Closing the root terminal
        // In a real app, you might want to exit the app here, but let's just recreate a new root.
        rootNode = TerminalNode(id: 'root');
        return;
      }
      
      TerminalNode parent = node.parent!;
      parent.children.remove(node);
      
      if (parent.children.length == 1) {
        // Only one child left, collapse
        TerminalNode remainingChild = parent.children.first;
        if (parent.parent == null) {
          rootNode = remainingChild;
          rootNode.parent = null;
        } else {
          TerminalNode grandParent = parent.parent!;
          int index = grandParent.children.indexOf(parent);
          grandParent.children[index] = remainingChild;
          remainingChild.parent = grandParent;
        }
      } else if (parent.children.isEmpty) {
        _closeNode(parent);
      }
    });
  }

  Widget _buildTree(TerminalNode node) {
    if (node.isLeaf) {
      return CustomTerminalView(
        key: ValueKey(node.id),
        initialCwd: node.initialCwd,
        onSplitHorizontally: (cwd) => _splitNode(node, Axis.horizontal, cwd),
        onSplitVertically: (cwd) => _splitNode(node, Axis.vertical, cwd),
        onClose: () => _closeNode(node),
      );
    }

    List<Widget> childrenWidgets = node.children.map((child) => _buildTree(child)).toList();
    MultiSplitViewController controller = MultiSplitViewController(
      areas: childrenWidgets.map((c) => Area(builder: (context, area) => c)).toList(),
    );

    return MultiSplitView(
      axis: node.direction,
      controller: controller,
    );
  }

  @override
  Widget build(BuildContext context) {
    return MultiSplitViewTheme(
      data: MultiSplitViewThemeData(
        dividerPainter: DividerPainters.grooved1(
          color: Colors.grey[800]!,
          highlightedColor: Colors.blue,
        ),
      ),
      child: _buildTree(rootNode),
    );
  }
}
