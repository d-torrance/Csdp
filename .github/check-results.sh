#!/bin/sh
#
# Check the output of "make unitTest" against the tolerances documented in
# INSTALL: the optimal objective values should match to at least six digits
# and all DIMACS errors should be smaller than 5.0e-7.
#
# The stock test/Makefile only writes the .out files and asks the user to
# eyeball them, which means no packager can currently detect a numerical
# regression.  This script is used by CI instead.
#
set -eu

status=0

check() {
	file=$1
	expected=$2

	printf '== %s (expecting %s)\n' "$file" "$expected"

	if [ ! -s "$file" ]; then
		echo "FAIL: $file is missing or empty"
		status=1
		return
	fi

	if ! grep -q 'Success: SDP solved' "$file"; then
		echo "FAIL: solver did not report success"
		sed -n '$p' "$file"
		status=1
		return
	fi

	if ! awk -v expected="$expected" '
	/^Primal objective value:/ { pobj = $4 + 0; havep = 1 }
	/^Dual objective value:/   { dobj = $4 + 0; haved = 1 }
	/^DIMACS error measures:/  {
		haved5 = 1
		for (i = 4; i <= NF; i++) {
			e = ($i + 0 < 0) ? -($i + 0) : $i + 0
			if (e > 5.0e-7) {
				printf "FAIL: DIMACS error %d is %s, want < 5.0e-7\n", \
					i - 3, $i
				bad = 1
			}
		}
	}
	function abs(x) { return x < 0 ? -x : x }
	END {
		scale = abs(expected) > 1 ? abs(expected) : 1
		if (!havep)  { print "FAIL: no primal objective value"; bad = 1 }
		if (!haved)  { print "FAIL: no dual objective value";   bad = 1 }
		if (!haved5) { print "FAIL: no DIMACS error measures";  bad = 1 }
		if (havep && abs(pobj - expected) / scale > 1.0e-6) {
			printf "FAIL: primal objective %.8e, want %.8e\n", pobj, expected
			bad = 1
		}
		if (haved && abs(dobj - expected) / scale > 1.0e-6) {
			printf "FAIL: dual objective %.8e, want %.8e\n", dobj, expected
			bad = 1
		}
		if (!bad)
			printf "ok: objectives %.8e / %.8e, DIMACS errors within tolerance\n", \
				pobj, dobj
		exit bad ? 1 : 0
	}' "$file"; then
		status=1
	fi
}

check test/theta1.out 23
check test/g50.out 23

exit $status
