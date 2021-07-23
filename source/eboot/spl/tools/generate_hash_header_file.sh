generate_hash_header(){
	local _commitHash=`git show HEAD --pretty=format:"%H" | head -n 1`
	local _dirty=`git describe --dirty|grep -o dirty$`
	if [ x${_dirty} = x ]
	then
		echo "#define CI_INFO \"$_commitHash\""
	else
		echo "#define CI_INFO \"$_commitHash-$_dirty\""
	fi
}

generate_hash_header
