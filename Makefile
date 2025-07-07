.PHONY: example tests

example:
	g++-10 -fsanitize=address example.cpp ./sdsl-lite/build/lib/libsdsl.a -std=c++20 -I sdsl-lite/include/ -O3 -I include -I ./sdsl-lite/build/external/libdivsufsort/include/ -g -o example -Wno-deprecated-declarations

tests:
	g++-10 tests.cpp ./sdsl-lite/build/lib/libsdsl.a -std=c++20 -I sdsl-lite/include/ -I include -I ./sdsl-lite/build/external/libdivsufsort/include/ -g -o tests -Wno-deprecated-declarations

queries:
	g++-10 gen_queries.cpp -std=c++20 -g -o gen_queries -Wno-deprecated-declarations

benchmark:
	g++-10 -fsanitize=address color_set_tests.cpp ./sdsl-lite/build/lib/libsdsl.a -std=c++20 -I sdsl-lite/include/ -O3 -I include -I ./sdsl-lite/build/external/libdivsufsort/include/ -g -o color_set_test -Wno-deprecated-declarations

benchmark_int:
	g++-10 color_set_tests.cpp ./sdsl-lite/build/lib/libsdsl.a -std=c++20 -I sdsl-lite/include/ -O3 -I include -I ./sdsl-lite/build/external/libdivsufsort/include/ -g -o color_set_test_int -Wno-deprecated-declarations

all_label:
	g++-10 all.cpp ./sdsl-lite/build/lib/libsdsl.a -std=c++20 -I sdsl-lite/include/ -O3 -I include -I ./sdsl-lite/build/external/libdivsufsort/include/ -g -o all_label -Wno-deprecated-declarations