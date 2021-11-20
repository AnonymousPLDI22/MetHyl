Step 1/5: Init Sampler
Finished
State type: List[10](List[10](Int[0,1000]))
Env type: []



Step 2/5: Synthesize PreOrder for Plan
Finished 0.019675
Cmp:[<=:let f = (\ x@(Int*Int) -> x.2) in sum(map(f,Param@0))]



Step 3/5: Rewrite F via PreOrders
Finished 3.516698
Cared Funtions
	let f = (\ x@(Int*Int) -> x.2) in sum(map(f,Param@0))
State: List[10](List[10](Int[0,1000]))	 Plan: Int
T: if (==(size(Param@0),1)) then collect(2,empty) else foreach step in ..(1,+(size(Param@0),-1)), let n = size(head(Param@0)) in let cur = -(n,size(Param@0)) in let next = +(cur,step) in collect(1,'(next,access(head(Param@0),next),drop(Param@0,step)));
F:
	collect(0,+(Param@1,Param@2))
	collect(0,0)



Step 4/5: Synthesize Eq Relation for States
Finished 0.181921
Eq:[lmatch(Param@0,Param@1)]



Step 5/5: Rewrite T via Eq Relation
Failed 14.467304
