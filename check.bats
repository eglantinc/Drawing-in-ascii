examples_folder=examples
prog=canvascii
help_first_line="Usage: ./canvascii [-n HEIGHT,WIDTH] [-s] [-k] [-p CHAR]"

# Normal usage

@test "With no argument, shows help" {
    run ./$prog
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "$help_first_line" ]
}

@test "Creating an empty 3x2 canvas" {
    run ./$prog -n 3,2
    [ "${lines[0]}" = ".." ]
    [ "${lines[1]}" = ".." ]
    [ "${lines[2]}" = ".." ]
}

@test "Maximum allowed size is 40x80" {
    run ./$prog -s < $examples_folder/empty40x80.canvas
    [ "$status" -eq 0 ]
}

@test "Loading and prints an empty 5x8 canvas" {
    run ./$prog -s < $examples_folder/empty5x8.canvas
    [ "$status" -eq 0 ]
}

@test "Using all pens between 0 and 7" {
    run ./$prog -s < $examples_folder/all-pens5x8.canvas
    [ "$status" -eq 0 ]
}

@test "Drawing horizontal line on 5x8 canvas with option -h" {
    run ./$prog -h 2 < examples/empty5x8.canvas
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "........" ]
    [ "${lines[1]}" = "........" ]
    [ "${lines[2]}" = "77777777" ]
    [ "${lines[3]}" = "........" ]
    [ "${lines[4]}" = "........" ]
}

@test "Drawing vertical line on 5x8 canvas with option -v" { 
    run ./$prog -v 3 < examples/empty5x8.canvas
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "...7...." ]
    [ "${lines[1]}" = "...7...." ]
    [ "${lines[2]}" = "...7...." ]
    [ "${lines[3]}" = "...7...." ]
    [ "${lines[4]}" = "...7...." ]
}

@test "Drawing rectangle on 5x8 canvas with option -r" {
    run ./$prog -r 1,2,3,4 < examples/empty5x8.canvas
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "........" ]
    [ "${lines[1]}" = "..7777.." ]
    [ "${lines[2]}" = "..7..7.." ]
    [ "${lines[3]}" = "..7777.." ]
    [ "${lines[4]}" = "........" ]
}

@test "Drawing line on 5x5 canvas with option -l" {
    run ./$prog -n 5,5 -l 0,0,4,4
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "7...." ]
    [ "${lines[1]}" = ".7..." ]
    [ "${lines[2]}" = "..7.." ]
    [ "${lines[3]}" = "...7." ]
    [ "${lines[4]}" = "....7" ]
}

@test "Drawing circle on 5x8 canvas with option -c" {
    run ./$prog -c 2,3,2 < examples/empty5x8.canvas
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "..777..." ]
    [ "${lines[1]}" = ".7...7.." ]
    [ "${lines[2]}" = ".7...7.." ]
    [ "${lines[3]}" = ".7...7.." ]
    [ "${lines[4]}" = "..777..." ]
}

@test "Combining multiple options" {
    run ./$prog -n 5,5 -p 1 -l 0,0,4,4 -p 7 -l 0,4,4,0 -p 2 -r 0,0,5,5
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "22222" ]
    [ "${lines[1]}" = "21.72" ]
    [ "${lines[2]}" = "2.7.2" ]
    [ "${lines[3]}" = "27.12" ]
    [ "${lines[4]}" = "22222" ]
}

@test "Drawing non diagonal segment" {
    run ./$prog -n 5,10 -l 1,1,3,8
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = ".........." ]
    [ "${lines[1]}" = ".77......." ]
    [ "${lines[2]}" = "...7777..." ]
    [ "${lines[3]}" = ".......77." ]
    [ "${lines[4]}" = ".........." ]
}

@test "Drawing large circle" {
    run ./$prog -n 11,11 -c 5,5,4
    [ "$status" -eq 0 ]
    [ "${lines[0]}"  = "..........." ]
    [ "${lines[1]}"  = "....777...." ]
    [ "${lines[2]}"  = "..77...77.." ]
    [ "${lines[3]}"  = "..7.....7.." ]
    [ "${lines[4]}"  = ".7.......7." ]
    [ "${lines[5]}"  = ".7.......7." ]
    [ "${lines[6]}"  = ".7.......7." ]
    [ "${lines[7]}"  = "..7.....7.." ]
    [ "${lines[8]}"  = "..77...77.." ]
    [ "${lines[9]}"  = "....777...." ]
    [ "${lines[10]}" = "..........." ]

}

@test "Clipping line from (1,1) to (5,8)" {
    run ./$prog -n 5,5 -l 1,1,5,8
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "....." ]
    [ "${lines[1]}" = ".7..." ]
    [ "${lines[2]}" = "..77." ]
    [ "${lines[3]}" = "....7" ]
    [ "${lines[4]}" = "....." ]
}

@test "Clipping circle of radius 3 centered at (3,3)" {
    run ./$prog -n 5,5 -c 3,3,3
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "..777" ]
    [ "${lines[1]}" = ".7..." ]
    [ "${lines[2]}" = "7...." ]
    [ "${lines[3]}" = "7...." ]
    [ "${lines[4]}" = "7...." ]
}

@test "Option -k is recognized" {
    run ./$prog -n 19,19 -p 1 -c 9,9,9 -l 3,3,15,15 -l 3,15,15,3 -k
    [ "$status" -eq 0 ]
}

# Handling errors

@test "Forbidding character # in canvas" {
    run ./$prog -s < $examples_folder/wrong-char.canvas
    [ "$status" -eq 1 ]
    [ "${lines[0]}" = "Error: wrong pixel value #" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Canvas of 41 lines is too high" {
    run ./$prog -s < $examples_folder/too-high.canvas
    [ "$status" -eq 2 ]
    [ "${lines[0]}" = "Error: canvas is too high (max height: 40)" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Canvas of 81 columns is too wide" {
    run ./$prog -s < $examples_folder/too-wide.canvas
    [ "$status" -eq 3 ]
    [ "${lines[0]}" = "Error: canvas is too wide (max width: 80)" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Width must be uniform for all lines" {
    run ./$prog -s < $examples_folder/non-uniform-width.canvas
    [ "$status" -eq 4 ]
    [ "${lines[0]}" = "Error: canvas should be rectangular" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Unrecognized option -a" {
    run ./$prog -n 5,5 -a
    [ "$status" -eq 5 ]
    [ "${lines[0]}" = "Error: unrecognized option -a" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Option -n must be provided with values" {
    run ./$prog -n
    [ "$status" -eq 6 ]
    [ "${lines[0]}" = "Error: missing value with option -n" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Wrong value with option -p" {
    run ./$prog -n 5,5 -p a
    [ "$status" -eq 7 ]
    [ "${lines[0]}" = "Error: incorrect value with option -p" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Wrong value with option -h" {
    run ./$prog -n 3,5 -h 6
    [ "$status" -eq 7 ]
    [ "${lines[0]}" = "Error: incorrect value with option -h" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Wrong value with option -v" {
    run ./$prog -n 3,5 -v 5
    [ "$status" -eq 7 ]
    [ "${lines[0]}" = "Error: incorrect value with option -v" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Wrong syntax with option -n" {
    run ./$prog -n 5x5
    [ "$status" -eq 7 ]
    [ "${lines[0]}" = "Error: incorrect value with option -n" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Wrong dimensions with option -n" {
    run ./$prog -n 80,40
    [ "$status" -eq 7 ]
    [ "${lines[0]}" = "Error: incorrect value with option -n" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

# Negative values

@test "Negative value with option -h is forbidden" {
    run ./$prog -n 5,10 -h -1
    [ "$status" -eq 7 ]
    [ "${lines[0]}" = "Error: incorrect value with option -h" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Negative value with option -v is forbidden" {
    run ./$prog -n 5,10 -v -1
    [ "$status" -eq 7 ]
    [ "${lines[0]}" = "Error: incorrect value with option -v" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Negative positions with option -r are allowed" {
    run ./$prog -n 5,10 -r -1,-1,4,4
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "..7......." ]
    [ "${lines[1]}" = "..7......." ]
    [ "${lines[2]}" = "777......." ]
    [ "${lines[3]}" = ".........." ]
    [ "${lines[4]}" = ".........." ]
}

@test "Negative dimensions with option -r are forbidden" {
    run ./$prog -n 5,10 -r 2,8,-3,5
    [ "$status" -eq 7 ]
    [ "${lines[0]}" = "Error: incorrect value with option -r" ]
    [ "${lines[1]}" = "$help_first_line" ]
}

@test "Negative positions with option -l are allowed" {
    run ./$prog -n 5,5 -l -1,-1,6,6
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "7...." ]
    [ "${lines[1]}" = ".7..." ]
    [ "${lines[2]}" = "..7.." ]
    [ "${lines[3]}" = "...7." ]
    [ "${lines[4]}" = "....7" ]
}

@test "Negative positions with option -c are allowed" {
    run ./$prog -n 5,5 -c -1,-1,5
    [ "$status" -eq 0 ]
    [ "${lines[0]}" = "....7" ]
    [ "${lines[1]}" = "....7" ]
    [ "${lines[2]}" = "...7." ]
    [ "${lines[3]}" = "..7.." ]
    [ "${lines[4]}" = "77..." ]
}

@test "Negative radius with option -c is forbidden" {
    run ./$prog -n 5,5 -c 1,1,-3
    [ "$status" -eq 7 ]
    [ "${lines[0]}" = "Error: incorrect value with option -c" ]
    [ "${lines[1]}" = "$help_first_line" ]
}


