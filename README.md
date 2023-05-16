<div dir="rtl">

<br>

<center style=font-family 'Tajawal'; font-size = 130px>


<img src="resources/AlifLogo/AlifLogo_2048x2048_transp.png"  width="150" height="150" >

</center>

<span style="font-family: 'Tajawal';">

<br>

# نبذة عن لغة ألف :

 لغة ألف هي لغة مفسرة ديناميكية تستند الى لغة <em> cpp </em> , تم انشاءها لاغراض عامة   
تعمل على نظام  <em> linux  </em>  و <em> Windows  </em> 

<br>

---

# طريقة بناء الشفرة

##

## نظام <em> Windows  </em> :

قم بتحميل برنامج <em> visual studio </em> اصدار 2014 فما فوق   
يمكنك تحميله من خلال الرابط : https://visualstudio.microsoft.com/downloads/  
قم بتحميل الشفرة المصدرية من مجتمع ألف على <em> github </em> : https://github.com/alifcommunity/Alif5  
ثم قم بفتح ملف <em> winBuild </em> الموجود على ملف <em> Alif5 </em> بعدها قم بتشغل ملف <em> Alif5.sln </em>  
سيتم تشغيل مشروع الف على برنامج <em> visual studio </em>  
يمكنك كتابة الشيفرة من خلال ملف <em> code.alif5 </em> الموجود في ملف <em> Alif5 </em>  

###### ملاحظة : يجب توفر حزمة  desktop development with c++ الموجودة على visual studio installer

##

## نظام <em> linux  </em> :

يجب توفر <em> g++ </em> إصدار 8.0.0 فما فوق  
	يمكنك تحميله من خلال هذا الرابط : https://github.com/niXman/mingw-builds-binaries/releases  
 قم بتحميل الشفرة المصدرية من مجتمع ألف على <em> github </em> : https://github.com/alifcommunity/Alif5  
 قم بفتح ملف <em> source </em> الموجود في ملف <em> Alif5 </em> 
 ومن ثم تقوم بتشغيل <em> cmd </em> في نفس مسار الملف  
 بعد ذلك تقوم بكتابة امر  :keyboard:
 
 ``` c++
 g++ Alif5.cpp AlifMemory.cpp AlifNamesTable.cpp Compiler.cpp Error.cpp Interpereter.cpp Lexer.cpp Parser.cpp -o alif5 
 ```
سيتم إنشاء ملف تنفيذي في نفس المسار باسم alif5.exe   
لتشغيل اللغة التفاعلية قم بكتابة امر 
 ``` c++
 alif5  
 ```
 سيتم تشغيل اللغة على وضع التفاعلي يمكنك كتابة الشفرة من خلاله   :man_technologist:
 
 وفي حال تنفيذ ملف خارجي قم بتمرير اسم الملف  

``` c++ 
alif5 filename.alif5  
```
