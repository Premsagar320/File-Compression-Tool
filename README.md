
# File Compression Tool (Huffman Coding)

This project is a simple file compression tool built in C++ using Huffman Coding. The goal was to understand how lossless compression works under the hood and to implement the full algorithm without relying on external libraries.

## What the Tool Does

The program reads an input file, analyzes the frequency of characters, builds a Huffman Tree, and then encodes the file using variable-length bit codes. The output is a compressed file that can be decompressed back to the original without any loss.

## How It Works (In Simple Terms)

1. Count how often each character appears.
2. Use a priority queue to build the Huffman Tree.
3. Assign shorter codes to frequent characters and longer codes to rare ones.
4. Write the compressed bitstream to a file.
5. The decompressor reverses the process using the stored tree information.

## Why This Project

The aim was to get hands-on experience with:
- data structures like priority queues and trees  
- bit manipulation  
- memory-efficient encoding  
- implementing a real compression algorithm from scratch  



