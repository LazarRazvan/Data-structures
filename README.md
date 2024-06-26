# Data-structures

Explore a comprehensive collection of C implementations for several fundamental data structures in this GitHub repository.

## Repository Structure

The repository is organized into two main directories: `include` and `src`. The `include` directory contains header files for each data structure, defining the interfaces and necessary structures. The `src` directory hosts the corresponding source files (`*.c`) implementing the functionality.

## Data Structures Implemented

- **Graphs (`graph`):**
  - Located in `include/graph` and `src/graph`.
  - Provides an implementation for graph data structures, useful for representing and manipulating networks and relationships.

- **Kernel Generic Doubly Linked List (`kdoubly_linked_list`):**
  - Found in `include/list` and `src/list`.
  - Implements a generic doubly linked list inspired by similar structures used in kernel development. It offers flexibility and efficiency in scenarios where bidirectional traversal is necessary.

- **Kernel Generic Singly Linked List (`ksingly_linked_list`):**
  - Available in `include/list` and `src/list`.
  - A generic singly linked list variant, again drawing inspiration from kernel-style data structures. It's optimized for scenarios where memory usage is a critical factor, and only forward traversal is needed.

- **Single Linked List (`singly_linked_list`):**
  - Located in `include/list` and `src/list`.
  - A straightforward implementation of a singly linked list, ideal for simple list management tasks in C.

- **Queue (`queue`):**
  - Files are in `include/queue` and `src/queue`.
  - Implements a queue data structure, essential for various computing scenarios like task scheduling, resource management, and breadth-first search algorithms.

- **Trie (`trie`):**
  - Found in `include/trie` and `src/trie`.
  - Provides an implementation of the trie data structure, excellent for efficient storage and retrieval of strings, particularly useful in autocomplete systems and IP routing.

- **Binary Tree (`binary_tree`):**
  - Located in `include/binary_tree` and `src/binary_tree`.
  - Implements a binary tree data structure, fundamental for various algorithmic tasks and data organization. This structure provides efficient operations for searching, insertion, and traversal, making it suitable for applications such as sorting and representing hierarchical data.

- **AVL Tree (`avl_tree`):**
  - Located in `include/avl_tree` and `src/avl_tree`.
  - The AVL tree is a self-balancing binary search tree where the heights of two child subtrees of any node differ by no more than one. Whenever the tree becomes unbalanced, as a result of standard BST operations such as insertions and deletions, AVL tree algorithms automatically perform rotations to restore its balance. This ensures that the tree maintains an O(log n) search time complexity, making it highly efficient for lookups, insertions, and deletions.
