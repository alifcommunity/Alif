# هذا البرنامج يقوم بتحويل ترميز الملفات الى utf-8
# يجب تشغيله بإستخدام PowerShell
# كتب من قبل shadow

$filePath = Read-Host -Prompt "ادخل مسار الملفات التي سيتم ترميزها "
$files = Get-ChildItem -Path $filePath -Recurse -Include *.cpp *.h
pause
Write-Host "تم قبول المسار، جاري تحويل الملفات ..."

foreach ($file in $files) {
    $content = Get-Content -Path $file.FullName -Encoding UTF8
    Set-Content -Path $file.FullName -Value $content -Encoding UTF8
}

Write-Host "تم تحويل جميع الملفات الى ترميز UTF-8"