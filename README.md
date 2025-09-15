# Jedi-Sith-Battle-Simulator

This project is a C++ battle simulator that models large-scale conflicts between Jedi and Sith across multiple planets. The program processes troop deployments, resolves battles using priority queues, and provides analytical summaries based on command-line options. It supports both deterministic input files and pseudo-randomly generated deployments for testing.

Key features include:

Priority-based battle resolution with custom comparators for Jedi and Sith troops

Median tracking of troop losses per planet using an efficient two-heap method

General performance evaluation, reporting deployment numbers, losses, and survival rates

“Movie watcher” mode to highlight the most dramatic ambushes and attacks per planet

Command-line parsing with getopt_long() for flexible output modes (verbose, median, general-eval, watcher)

Robust input validation and error handling using std::cin, std::cout, and std::cerr

This project demonstrates algorithmic simulation design, custom data structures, efficient statistics tracking, and practical systems programming in C++.
