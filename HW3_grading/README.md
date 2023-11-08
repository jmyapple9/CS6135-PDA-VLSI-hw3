# Homework 3: Grading Script
We will use this script to grade your program.  
**Please make sure your program can be executed by this script.**

## Preparing
* Step1:  
    Go into directory `student` and generate a new directory with your student id.
    ```sh
    $ cd student/
    $ mkdir ${your_student_id}
    ```

    E.g.,
    ```sh
    $ cd student/
    $ mkdir 112062500
    ```

* Step2:  
    Put your compressed file in the directory which you just generated.  
    The whole path is as follow: 
    ```
    HW3_grading/student/${your_student_id}/CS6135_HW3_${your_student_id}.tar.gz
    ```

    E.g.,
    ```
    HW3_grading/student/112062500/CS6135_HW3_112062500.tar.gz
    ```

### Notice:  
**Do not put your original directory here** because it will remove all directories before unzipping the compressed file.

## Working Flow
* Step1:  
    Go into directory `HW3_grading` and run `HW3_grading.sh`.
    ```sh
    $ cd HW3_grading/
    $ bash HW3_grading.sh
    ```

* Step2:  
    Check your output.
    * If the status is **success**, it means your program finished in time and your output is correct. E.g.,
        ```
        grading on 112062500:
          testcase | wirelength |    runtime | status
           public1 |  157924052 |     590.71 | success
           public2 |   19610550 |     591.34 | success
           public3 |    1882293 |     591.40 | success
           public4 |   63614225 |     591.46 | success
        ```
    * If the status is not **success**, it means your program failed in this case. E.g.,
        ```
        grading on 112062500:
          testcase | wirelength |    runtime | status
           public1 |       fail |        TLE | public1 failed.
           public2 |       fail |        TLE | public2 failed.
           public3 |       fail |        TLE | public3 failed.
           public4 |       fail |        TLE | public4 failed.
        ```
