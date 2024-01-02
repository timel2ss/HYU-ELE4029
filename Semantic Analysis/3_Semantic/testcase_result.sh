#!/bin/bash

make clean
make all

# 결과를 저장할 디렉토리 설정
result_directory="./my_result"

# cminus_semantic 실행 파일 확인
cminus_executable="./cminus_semantic"
if [ ! -x "$cminus_executable" ]; then
    echo "cminus_semantic 실행 파일이 현재 디렉토리에 없거나 실행 권한이 없습니다."
    exit 1
fi

# 현재 디렉토리 및 하위 디렉토리에서의 *.cm 파일 목록 가져오기
cm_files=($(find . -type f -name "*.cm"))

# 결과를 저장할 디렉토리 생성
rm -rf "$result_directory"
mkdir -p "$result_directory"

# 각 *.cm 파일에 대해 cminus_semantic 실행 및 결과 저장
for file in "${cm_files[@]}"; do
    output_file="$result_directory/$(basename ${file%.*}).result"
    ./"$cminus_executable" "$file" > "$output_file"
    echo "File '$file'의 실행 결과는 '$output_file'에 저장되었습니다."
done