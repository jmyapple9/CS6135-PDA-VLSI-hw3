#!/bin/bash
homeworkName=HW3
binaryName=hw3
testcasePool="public1 public2 public3 public4"
timeLimit=600

root=$(pwd)
outputDir=$root/output
studentDir=$root/student
testcaseDir=$root/testcase
verifierBin=$root/verifier/verify
chmod +x $verifierBin
csvFile=$root/${homeworkName}_grade.csv

function executionCmd() {
	local argv="$testcaseDir/$1.txt $outputDir/$1.floorplan"
	local log=$(timeout $timeLimit time -p ./$binaryName $argv 2>&1 >/dev/null)
	if [[ $log =~ "real " ]] && [[ $log =~ "user " ]] && [[ $log =~ "sys " ]]; then
		echo "$(echo "$log" | grep real | tail -1 | cut -d ' ' -f 2)"
	else
		echo "TLE"
	fi
}

function verifyCmd() {
	local argv="$testcaseDir/$1.txt $outputDir/$1.floorplan"
	local log=$($verifierBin $argv | cat)
	if [[ $log =~ "[Success]" ]]; then
		echo "success"
	elif [[ $log =~ "[Error] Wrong Number!" ]]; then
		echo "$1 has the wrong soft module number."
	elif [[ $log =~ "[Error] Missing Soft Module!" ]]; then
		echo "Some soft modules in $1 are missing."
	elif [[ $log =~ "[Error] Duplicated Soft Module!" ]]; then
		echo "Some soft modules in $1 are duplicated."
	elif [[ $log =~ "[Error] Constraint Violated! The area" ]]; then
		echo "Some soft modules in $1 violate the min area constraint."
	elif [[ $log =~ "[Error] Constraint Violated! The aspect" ]]; then
		echo "Some soft modules in $1 violate the aspect ratio constraint."
	elif [[ $log =~ "is not in the chip boundary" ]]; then
		echo "Some soft modules in $1 are not in the chip boundary."
	elif [[ $log =~ "overlaps with soft module" ]]; then
		echo "Some soft modules in $1 overlap with other soft modules."
	elif [[ $log =~ "overlaps with fixed module" ]]; then
		echo "Some soft modules in $1 overlap with other fixed modules.."
	elif [[ $log =~ "[Error] Wrong Wirelength!" ]]; then
		echo "$1 has wrong wirelength."
	else
		echo "$1 has some errors."
	fi
}

echo "|------------------------------------------------|"
echo "|                                                |"
echo "|    This script is used for PDA $homeworkName grading.    |"
echo "|                                                |"
echo "|------------------------------------------------|"

csvTitle="student id"
for testcase in $testcasePool; do
	csvTitle="$csvTitle, $testcase wirelength, $testcase runtime"
done
echo "$csvTitle, status" >$csvFile

cd $studentDir/
for studentId in *; do
	if [[ -d $studentId ]]; then
		printf "grading on %s:\n" $studentId
		cd $studentId/
		rm -rf $(find . -mindepth 1 -maxdepth 1 -type d)
		tar -zxf CS6135_${homeworkName}_$studentId.tar.gz
		cd $homeworkName/

		cd src/
		make clean >/dev/null
		rm -rf ../bin/*
		make >/dev/null
		cd ..

		cd bin/
		rm -rf $outputDir/*
		csvContent="$studentId"
		statusList=""
		printf "%10s | %10s | %10s | %s\n" testcase wirelength runtime status
		for testcase in $testcasePool; do
			printf "%10s | " $testcase
			wirelength=fail
			status="$testcase failed."
			runtime=$(executionCmd $testcase)
			if [[ $runtime != "TLE" ]]; then
				status=$(verifyCmd $testcase)
				if [[ $status == "success" ]]; then
					wirelength=$(cat $outputDir/$testcase.floorplan | grep Wirelength | cut -d ' ' -f 2)
				else
					statusList="$statusList $status"
				fi
			else
				statusList="$statusList $status"
			fi
			printf "%10s | %10s | %s\n" $wirelength $runtime "$status"
			csvContent="$csvContent, $wirelength, $runtime"
		done
		echo "$csvContent, $statusList" >>$csvFile

		cd $studentDir/
	fi
done

echo "|-----------------------------------------------------|"
echo "|                                                     |"
echo "|    Successfully generate grades to ${homeworkName}_grade.csv    |"
echo "|                                                     |"
echo "|-----------------------------------------------------|"
