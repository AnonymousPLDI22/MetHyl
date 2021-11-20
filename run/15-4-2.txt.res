Step 1/5: Init Sampler
Finished
State type: List[10](Int[0,5])
Env type: [Int[5,10]]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.069526
Cmp:[<=:if (==(size(Param@0),0)) then 0 else let f = (\ x@(Int*Int) -> pow(-(-(Param@1,x.1),x.2),3)) in let rem = reverse(tail(reverse(Param@0))) in neg(sum(map(f,rem)));]



Step 3/5: Rewrite F via PreOrders
Failed 120.239962



Step 4/5: Synthesize Eq Relation for States
Finished 0.435002
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Failed 5.782650
