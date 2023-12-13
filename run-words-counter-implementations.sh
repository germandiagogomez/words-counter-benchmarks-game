#!/usr/bin/env sh

build-release/programs/words_counter -D wikipedia_resources/ -n 1024 -i MonoThreadWordsCounter -o
build-release/programs/words_counter -D wikipedia_resources/ -n 1024 -i AsyncWordsCounter -o
build-release/programs/words_counter -D wikipedia_resources/ -n 1024 -i ExecutorBasedFutureWordsCounter -o
build-release/programs/words_counter -D wikipedia_resources/ -n 1024 -i BoostFutureMemMappedFileWordsCounter -o -a split_words_pool_size=1 -a count_words_pool_size=2 -a merge_results_pool_size=1

mkdir -p words-counter-results && mv *.out words-counter-results/
