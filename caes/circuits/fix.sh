#!/bin/bash
t1=${1}
t2=tmp.txt

function doit() {
   cat ${t1} | sed -r ${1}  > ${t2}
   mv ${t2} ${t1}
}

doit 's/size_of_heap\((.*)\);/mm->init_heap(\1);/'

doit 's/no_of_ands\(.*\);//'
doit 's/max_width_XOR\(.*\);//'
doit 's/max_width_AND\(.*\);//'
doit 's/max_width_private_common_load\(.*\);//'
doit 's/max_width_public_common_load\(.*\);//'
doit 's/max_width_public_common_store\(.*\);//'
doit 's/max_width_INV\(.*\);//'

doit 's/begin_layer_AND\((.*)\);/mm->mulpar(\1);/'
doit 's/begin_.*\(.*\);//'
doit 's/end_.*\(.*\);//'

## private_common_load
doit \
's/private_common_load\((.*),(.*),(.*)\);/replica_private_input(\1,\2,mm);/'

# public_common_load
doit \
's/public_common_load\((.*),(.*),(.*)\);/replica_public_input(\1,\2,mm);/'

# XOR/ADD
doit 's/XOR\((.*),(.*),(.*),(.*)\);/mm->add(\1,\2,\3);/'

# AND/MUL
doit \
's/AND\((.*),(.*),(.*),(.*),(.*)\);/mm->mul(\1,\2,\3);/'

# INV/ADD ONE
doit 's/INV\((.*),(.*),(.*)\);/mm->add(\1,\2,ONE);/'

# public_common_store
doit 's/public_common_store\((.*),(.*),(.*)\);/mm->open(\2);/'
