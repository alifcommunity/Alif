<div dir="rtl">

<br>

<center style=font-family 'Tajawal'; font-size = 130px>


<img src="resources/AlifLogo/AlifLogo_2048x2048_transp.png"  width="150" height="150" >

</center>

<span style="font-family: 'Tajawal';">

<br>

# نُبذة عن لغةِ أَلِفْ :

لغة البرمجة أَلِفْ هي لغةٌ مُفسرة تكيفية تستند إلى لغة <em> cpp </em> , تم إنشاؤها لأغراض عامة.   
وتعمل على نِظامَي  <em> linux  </em>  وكذلك <em> Windows  </em> 

<br>

---

# طريقة بناء الشفرة

##

## نظام <em> Windows  </em> :

قُمْ بِتَحميل برنامج <em> visual studio </em> الإصدار 2014 وَمَا بَعدهُ.   
يمكنك تحميلهُ من خلالِ الرابط : https://visualstudio.microsoft.com/downloads/  
وبعدها قم بتحميلِ الشفرةِ المصدرية من مجتمع أَلِفْ على <em> github </em> : https://github.com/alifcommunity/Alif5  
ثم قم بفتح ملف <em> winBuild </em> الموجود في ملف <em> Alif5 </em> وبعدها قم بتشغيل ملف <em> Alif5.sln </em>  
سيتم تشغيل مشروع أَلِفْ على برنامج <em> visual studio </em>  
يمكنك كتابة الشيفرة من خلال ملف <em> code.alif5 </em> الموجود في ملف <em> Alif5 </em>  

###### ملاحظة : يجب أن تتوفر حزمة  desktop development with c++ الموجودة على visual studio installer

##

## نظام <em> linux  </em> :

يجب أن يتوفرالإصدار 8.0.0 فما فوق من <em> g++ </em>   
	يمكنك تحميله من خلال هذا الرابط : https://github.com/niXman/mingw-builds-binaries/releases  
 قم بتحميل الشفرة المصدرية من مجتمع ألف على <em> github </em> : https://github.com/alifcommunity/Alif5  
 قم بفتح ملف <em> Alif5 </em> 
 ومن ثم تقوم بتشغيل <em> cmd </em> في نفس مسار الملف.  
 بعد ذلك تقوم بكتابة أمر  :keyboard:
 
 ```sh
 make
 ```
سيتم إنشاء ملف تنفيذي في نفس المسار باسم alif5   
لتشغيل اللغة التفاعلية قم بكتابة أمر 
 ```sh
 ./alif5  
 ```
 سيتم تشغيل اللغة على وضع تفاعلي يمكنك كتابة الشفرة من خلاله   :man_technologist:
 
 وفي حالة تنفيذ من أجل ملف خارجي، قم بتمرير اسم الملف.  

```sh
alif5 filename.alif5  
```
