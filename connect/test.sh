start=$SECONDS
while true;
    do  
        duration=$(( SECONDS - start ))
        if [ $duration -gt 3 ];
            then
                echo "chiuso"
                break
            fi
    done