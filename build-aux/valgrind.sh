export TESTS_ENVIRONMENT="
	${VALGRIND:-valgrind} \
		--tool=memcheck \
		--leak-check=full \
		--leak-resolution=high \
		--num-callers=20 \
		--error-exitcode=1 \
		--show-possibly-lost=no"

if [[ "${1}" == *.sh ]]; then
    exec "${@}"
else
    exec ${TESTS_ENVIRONMENT} "${@}"
fi
