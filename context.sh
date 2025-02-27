OUTPUT=$1
rm -rf "${OUTPUT}"

FILES=`find . -name "*.c" -o -name "*.h" -o -name "*.md" -o -name "Makefile"`
while IFS= read -r FILE; do
  printf "FILE: ${FILE}\n" >> "${OUTPUT}"
  cat "${FILE}" >> "${OUTPUT}"
  printf -- "---\n\n" >> "${OUTPUT}"
done <<< "${FILES}"