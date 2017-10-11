#!bash -uvx
function sjis {
	$* |&  iconv -f cp932 -t utf-8
}
sjis mdtool build -c:Release b.sln

