Step 1/5: Init Sampler
Finished
State type: List[9](Int[0,4])
Env type: [Int[4,9]]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.209333
Cmp:[<=:if (==(size(Param@0),0)) then 0 else let f = (\ x@(Int*Int) y@Int -> ite(||(>(y,10000),>(+(x.1,x.2),Param@1)),10001,+(y,pow(-(-(Param@1,x.1),x.2),3)))) in let rem = reverse(tail(reverse(Param@0))) in let last = head(reverse(Param@0)) in neg(ite(>(+(last.1,last.2),Param@1),10001,fold(f,0,rem)))]



Step 3/5: Rewrite F via PreOrders
Failed 120.346162



Step 4/5: Synthesize Eq Relation for States
Finished 0.378727
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Failed 7.601102
