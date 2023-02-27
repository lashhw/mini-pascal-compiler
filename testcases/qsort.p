PROGRAM qsort(void);

    VAR

        i, j, tmp, size: integer;
    VAR

        arr: ARRAY [1..50] OF Integer;



    PROCEDURE readArr(VAR a: ARRAY [1..50] OF Integer);

    VAR input: integer;

        BEGIN

            size := 0;

            input := readlnI;

            WHILE input != 0 DO

            BEGIN

                size := size + 1;

                arr[size] := input;

                input := readlnI;

            END;


        END;


    FUNCTION split(start, stop: integer): integer;
    VAR
            test: integer;
        VAR
            left, right: integer;
    VAR
            pivot: integer;
    VAR
            tmp: integer;

        BEGIN

            pivot := arr[start];


            left := start + 1;

            right := stop;

            WHILE left <= right DO
            BEGIN
        test:=1;
        if left <= stop THEN
        ELSE
            test:=0;
        if test = 1 THEN
            if arr[left] < pivot THEN
            ELSE
                test := 0
        ELSE BEGIN END;

                WHILE test = 1 DO
        BEGIN
                left := left + 1;
            test:=1;
            if left <= stop THEN
            ELSE
                test:=0;
            if test = 1 THEN
                if arr[left] < pivot THEN
                ELSE
                    test := 0
            ELSE BEGIN END;
        END;

        test:=1;
        if right > start THEN
        ELSE
            test:=0;
        if test = 1 THEN
            if arr[right] >= pivot THEN
            ELSE
                test := 0
        ELSE BEGIN END;

                WHILE test = 1 DO
        BEGIN
                    right := right - 1;
            test:=1;
            if right > start THEN
            ELSE
                test:=0;
            if test = 1 THEN
                if arr[right] >= pivot THEN
                ELSE
                    test := 0
            ELSE BEGIN END;
        END;

                IF left < right THEN
        BEGIN

                    tmp :=arr[left];

                    arr[left] := arr[right];

                    arr[right] := tmp;

        END
        ELSE BEGIN END;

            END;

            tmp :=arr[start];

            arr[start] := arr[right];

            arr[right] := tmp;

            split := right

        END;

    PROCEDURE quicksortRecur(start, stop: integer);

        VAR

            m: integer;
        VAR

            splitpt: integer;

        BEGIN

            IF start < stop THEN
            BEGIN

                splitpt := split(start, stop);


                quicksortRecur(start, splitpt-1);

                quicksortRecur(splitpt+1, stop);

            END
            ELSE
            BEGIN
            END

        END;

    PROCEDURE quicksort(VAR arr: ARRAY [1..50] OF Integer);

        BEGIN


            quicksortRecur(1, size)

        END;


    BEGIN

        readArr(arr);


        quicksort(arr);

        i := 1;

        WHILE i <= size DO

        BEGIN

            writelnI(arr[i]);

            i := i+1;

        END

    END.
