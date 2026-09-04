#!/usr/bin/env bash
# تشغيل ما بُني - يعمل على لينكس وماك وويندوز (Git Bash).
#
# البناء الأخضر يثبت أن الشيفرة تُرجمت، لا أن المفسّر يعمل. وعلة الترميز
# بالذات قد تمر من الترجمة ثم تظهر عند التشغيل: اسم ملف لا يُفتح، أو نص
# عربي يُطبع مشوهاً. لذا نشغّل الأمثلة، ونكتب ملفاً عربي الاسم والمحتوى
# ونتحقق من مخرجه.
#
# الاستعمال: bash .github/scripts/run_check.sh <مسار مفسّر ألف>
set -u

[ $# -ge 1 ] || { echo "الاستعمال: $0 <مسار مفسّر ألف>" >&2; exit 2; }

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILT="$ROOT/$1"
[ -f "$BUILT" ] || { echo "لم يُعثر على المفسّر: $BUILT" >&2; exit 2; }

# نجهّز حزمة كما تُشحن: المفسّر ومكتبته جنباً إلى جنب. تشغيله من مجلد البناء
# مباشرة يخرج على ويندوز بالرمز 1 بلا رسالة، لأنه لا يجد library بجواره.
PKG="$(mktemp -d)"
trap 'rm -rf "$PKG"' EXIT
cp "$BUILT" "$PKG/"
cp -r "$ROOT/library" "$ROOT/examples" "$PKG/"
ALIF="$PKG/$(basename "$BUILT")"
chmod +x "$ALIF" 2>/dev/null || true

fails=0
pass() { echo "  [ناجح] $1"; }
fail() { echo "  [فاشل] $1"; fails=$((fails + 1)); }

# إدخال سخي يكفي الأمثلة التفاعلية (عدد للأولية، وحركات للعبة XO)
FEED="$(printf '30\n4\n0\n8\n2\n6\n1\n3\n5\n7\n')"

echo "== المفسّر: $ALIF"
echo

echo "== 1. تشغيل الأمثلة المتعقَّبة في git"
# نقتصر على المتعقَّب: تشغيل Status ينشئ ملف بيانات لا برنامجاً، فلو أخذناه
# بمحرف البدل لفشل في الجولة التالية.
EXAMPLES="$(cd "$ROOT" && git -c core.quotepath=false ls-files 'examples/*.alif' | sed 's|^examples/||')"
cd "$PKG" || exit 2
while IFS= read -r f; do
	[ -n "$f" ] && [ -e "examples/$f" ] || continue
	printf '%s' "$FEED" | "$ALIF" "examples/$f" > out.txt 2>&1
	rc=$?
	if [ $rc -eq 0 ]; then
		pass "$f"
	else
		fail "$f (رمز الخروج $rc)"
		tail -5 out.txt
	fi
done <<EOF
$EXAMPLES
EOF
rm -f out.txt
echo

echo "== 2. ملف عربي الاسم والمحتوى"
# هذا هو الاختبار الذي يمس الرقعة مباشرة: اسم الملف عربي، والنص المطبوع
# عربي. فإن ضاع الترميز في أي موضع - قراءة الاسم أو ترجمة السلسلة أو
# كتابتها إلى المخرج - ظهر هنا ولم يظهر في البناء.
TMP="$(mktemp -d)"
printf 'اطبع("سليم")\n' > "$TMP/تجربة.alif"
"$ALIF" "$TMP/تجربة.alif" > "$TMP/out.txt" 2>&1
rc=$?
if [ $rc -eq 0 ] && grep -q "سليم" "$TMP/out.txt"; then
	pass "اسم عربي، ونص عربي يُطبع كما كُتب"
else
	fail "رمز الخروج $rc"
	cat "$TMP/out.txt"
fi
rm -rf "$TMP"
echo

echo "== عدد الاختبارات الفاشلة: $fails"
[ "$fails" -eq 0 ]
