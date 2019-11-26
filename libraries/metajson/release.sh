#! /bin/bash

#ssh-agent zsh
ssh-add

git push
cd ../..
git commit -a -m 'Update libs.'
git push

cd libraries/metajson
rm metajson.hh
./make_single_header.py ./metajson.hh
git commit -a -m 'Update single header.'
git push
