#!/bin/sh
#
# The patches in this directory are a second rendering of commits that are
# already on this branch: the hand written DEP-3 header replaces documentation
# that the commit puts in INSTALL.  That means the diff below the header goes
# stale as soon as the commit is amended, so regenerate it from the commit
# rather than editing it by hand.
#
#     ./patches/regenerate.sh            rewrite the patch bodies
#     ./patches/regenerate.sh --check    fail if any is out of date
#
set -e

cd "$(dirname "$0")/.."

check=no
[ "$1" = "--check" ] && check=yes

status=0

regenerate() {
	file=patches/$1
	subject=$2
	shift 2

	commit=$(git log --format=%H --grep="$subject" -n 1)
	if [ -z "$commit" ]; then
		echo "$file: no commit matching /$subject/" >&2
		exit 1
	fi

	new=$(mktemp)
	awk '/^diff --git/ { exit } { print }' "$file" > "$new"
	git diff "$commit^" "$commit" -- . "$@" >> "$new"

	if [ "$check" = yes ]; then
		if cmp -s "$file" "$new"; then
			echo "$file: up to date"
		else
			echo "$file: OUT OF DATE with $(git log --format=%h -n 1 "$commit")" >&2
			diff -u "$file" "$new" | head -40 >&2
			status=1
		fi
		rm -f "$new"
	else
		mv "$new" "$file"
		echo "$file: regenerated from $(git log --format=%h -n 1 "$commit")"
	fi
}

# The INSTALL changes are deliberately left out: that material lives in the
# DEP-3 header instead.
regenerate 0002-make-the-build-configurable.patch \
	'^Make the build configurable instead of hardcoded' \
	':(exclude)INSTALL'

exit $status
