#pgn-extract.exe mrdodgy21.pgn -Wuci --notags --output extracted.pgn
$files = ''
$FOLDER = "games"
$OUTPUT = "output"
$OUTPUT_FILE = "extracted.pgn"
$MOVES_OUTPUT_FILE = "openings.json"
$AMOUNT_OF_MOVES = 10
Write-Output "----------------------TASK: SEARCH FOR PGN FILES-------------------"
Get-ChildItem $FOLDER -Filter *.pgn | 
Foreach-Object {
  $files += $FOLDER + "/" + $_.Name + ' '
}
Write-Output "----------------------COMPLETED: SEARCH FOR PGN FILES--------------"
Write-Output "Files to scan for: " $files
Write-Output "----------------------TASK: EXTRACT PGN FILES----------------------"
Invoke-Expression ".\pgn-extract.exe ${files} -Wuci --notags --output ${OUTPUT}/${OUTPUT_FILE}" >$null 2>$null
Write-Output "----------------------COMPLETED: EXTRACT PGN FILES-----------------"
Write-Output "Extraction stored in ${OUTPUT}/${OUTPUT_FILE}"
Write-Output "----------------------TASK: CREATE MOVES DICTIONARY-------------------"
Invoke-Expression "python .\setup_opening.py -i ${OUTPUT}/${OUTPUT_FILE} -o ${OUTPUT}/${MOVES_OUTPUT_FILE} -m ${AMOUNT_OF_MOVES}"
Write-Output "----------------------COMPLETED: CREATE MOVES DICTIONARY--------------"
Write-Output "Moves stored in ${OUTPUT}/${MOVES_OUTPUT_FILE}"
#& ".\pgn-extract.exe ${files} -Wuci --notags --output extracted.pn"