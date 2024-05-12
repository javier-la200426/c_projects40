#!/bin/sh



echo "Running tests..."



# Find all UM test files

test_files=$(find . -maxdepth 1 -name "*.um")



for curr_file in $test_files; do

    echo "Running test $curr_file"


    

    # Extract test name from curr_file path

    test_name=$(echo "$curr_file" | sed -E 's/(.*).um/\1/')

    

    # Check if corresponding output curr_file (.1) exists

    output_file="$test_name.1"

    input_file="$test_name.0"

    if [ -f "$output_file" ]; then

        if [ -f "$input_file" ]; then

            ./um "$curr_file" < "$input_file" > "$test_name.out"
            um "$curr_file" < "$input_file" > "$test_name.gt"


        else

            ./um "$curr_file" > "$test_name.out"
            um "$curr_file" > "$test_name.gt"

        fi


        #comparing our um output with our expected output
        diff "$test_name.out" "$output_file" 
        #comparing our um output with reference um output 
        diff "$test_name.out" "$test_name.gt"
       

        if [ $? -eq 0 ]; then

            echo "Test $curr_file passed!"
            echo "" 

        else

            echo "Test $curr_file failed!"
            echo "" 

        fi

    else
        # #not expected output file 
        # if [ -f "$input_file" ]; then
        #     ./um "$curr_file" < "$input_file" > "$test_name.out"
        #     um "$curr_file" < "$input_file" > "$test_name.gt"
        # else
        # ./um "$curr_file" > "$test_name.out"
        # um "$curr_file" > "$test_name.gt"
        # diff "$test_name.out" "$test_name.gt"

        # if [ $? -eq 0 ]; then

        #     echo "Test $curr_file passed!"
        #     echo "" 

        # else

        #     echo "Test $curr_file failed!"
        #     echo "" 

        # fi
            


        # fi


        echo "$output_file does not exist! Skipping test $curr_file"
        echo "" 

    fi
    

done