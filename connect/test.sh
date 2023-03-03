 logs=()
    while read target;
        do
            #salvo i file di log in un array
            logs+=("log_$target.txt")
        done < target.txt

    #resetto i log
    for log in ${logs[@]};
        do
            > ./log_files/$log
        done

