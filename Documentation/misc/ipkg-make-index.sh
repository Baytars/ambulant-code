#!/bin/sh
#set -e

pkg_dir=${1:-.}
if [ -z $pkg_dir ] || [ ! -d $pkg_dir ]; then
  echo "Usage: ipkg-make-index <package_directory>"
  exit 1
fi

for pkg in `find $pkg_dir -name '*.ipk' -maxdepth 1 | sort`; do
  filename=`basename $pkg`
  pkgname=${filename%%_*}
  echo "Generating index for package ${pkgname}" >&2
  ## checking for multiple versions
  oldflag=
  for other in ${pkg_dir}/${pkgname}_*; do
    if [ $pkg != $other -a $other -nt $pkg ]; then
      oldflag=y
      break;
    fi;
  done
  if [ -z "$oldflag" ]; then \
    file_size=$(ls -l $pkg | awk '{print $5}')
    md5sum=$(md5sum $pkg | awk '{print $1}')
    # Take pains to make variable value sed-safe
    sed_safe_pkg=`echo $filename | sed -e 's/\\//\\\\\\//g'`
    file $pkg |grep gzip >/dev/null
    if [ $? -eq 0 ]; then 
    tar -xzOf $pkg ./control.tar.gz | tar xzOf - ./control | sed -e "s/^Filename:.*//g" | grep -v '^$' | sed -e "s/^Description:/Filename: $sed_safe_pkg\\
Size: $file_size\\
MD5Sum: $md5sum\\
Description:/"
    echo ""
    else
      (ar x $pkg control.tar.gz ; cat control.tar.gz ; rm control.tar.gz) | tar xzOf - ./control | sed -e "s/^Filename:.*//g" | grep -v '^$' | sed -e "s/^Description:/Filename: $sed_safe_pkg\\
Size: $file_size\\
MD5Sum: $md5sum\\
Description:/"
    echo ""
    fi
  else
    echo >&2 "Skipped old file: $pkg ($other is newer)"
    mv $pkg $pkg_dir/old/
  fi
done >> $pkg_dir/Packages


cat <<EOF >$pkg_dir/HEADER.html

<pre>
<b>This is a proper feed directory!</b>  You may use the URL for this location in a package manager ...
Note:  These are mirred files and may not be the most recent.  Please always seek the original source feeds, when possible.
</pre>


EOF
