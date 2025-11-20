#!/bin/bash
# Script to test compilation and basic functionality

echo "========================================="
echo "GSEA - Build and Test Script"
echo "========================================="
echo ""

# Clean previous build
echo "1. Cleaning previous build..."
make clean
echo ""

# Compile project
echo "2. Compiling project..."
make
if [ $? -ne 0 ]; then
    echo "❌ Compilation failed!"
    exit 1
fi
echo "✓ Compilation successful!"
echo ""

# Create output directory for tests
mkdir -p output

# Test 1: Compress
echo "3. Testing compression..."
./bin/gsea -c -i test/test_data.txt -o output/test1.huf
if [ $? -ne 0 ]; then
    echo "❌ Compression failed!"
    exit 1
fi
echo "✓ Compression successful!"
echo ""

# Test 2: Decompress
echo "4. Testing decompression..."
./bin/gsea -d -i output/test1.huf -o output/test1_restored.txt
if [ $? -ne 0 ]; then
    echo "❌ Decompression failed!"
    exit 1
fi
echo "✓ Decompression successful!"
echo ""

# Test 3: Verify integrity
echo "5. Verifying data integrity..."
diff test/test_data.txt output/test1_restored.txt
if [ $? -ne 0 ]; then
    echo "❌ Data integrity check failed!"
    exit 1
fi
echo "✓ Data integrity verified!"
echo ""

# Test 4: Encryption
echo "6. Testing encryption..."
./bin/gsea -e -i test/test_data.txt -o output/test2.aes -k test123
if [ $? -ne 0 ]; then
    echo "❌ Encryption failed!"
    exit 1
fi
echo "✓ Encryption successful!"
echo ""

# Test 5: Decryption
echo "7. Testing decryption..."
./bin/gsea -u -i output/test2.aes -o output/test2_restored.txt -k test123
if [ $? -ne 0 ]; then
    echo "❌ Decryption failed!"
    exit 1
fi
echo "✓ Decryption successful!"
echo ""

# Test 6: Verify encryption integrity
echo "8. Verifying encryption integrity..."
diff test/test_data.txt output/test2_restored.txt
if [ $? -ne 0 ]; then
    echo "❌ Encryption integrity check failed!"
    exit 1
fi
echo "✓ Encryption integrity verified!"
echo ""

# Test 7: Combined operations
echo "9. Testing combined compress+encrypt..."
./bin/gsea -ce -i test/test_data.txt -o output/test3.secure -k mypassword
if [ $? -ne 0 ]; then
    echo "❌ Combined compression+encryption failed!"
    exit 1
fi
echo "✓ Combined compression+encryption successful!"
echo ""

# Test 8: Reverse combined operations
echo "10. Testing combined decrypt+decompress..."
./bin/gsea -ud -i output/test3.secure -o output/test3_restored.txt -k mypassword
if [ $? -ne 0 ]; then
    echo "❌ Combined decryption+decompression failed!"
    exit 1
fi
echo "✓ Combined decryption+decompression successful!"
echo ""

# Test 9: Verify combined integrity
echo "11. Verifying combined operation integrity..."
diff test/test_data.txt output/test3_restored.txt
if [ $? -ne 0 ]; then
    echo "❌ Combined operation integrity check failed!"
    exit 1
fi
echo "✓ Combined operation integrity verified!"
echo ""

# Summary
echo "========================================="
echo "✓ ALL TESTS PASSED!"
echo "========================================="
echo ""
echo "Files generated:"
ls -lh output/
echo ""
echo "The program is working correctly!"

