Step 1/5: Init Sampler
Finished
State type: (List[6](Int[1,3])*List[6](Int[1,3]))
Env type: [List[6](Int[1,3])]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.103900
Cmp:[<=:let f = (\ x@Int -> access(Param@1,x)) in neg(sum(map(f,Param@0)))]



Step 3/5: Rewrite F via PreOrders
Finished 24.457509
Cared Funtions
	let f = (\ x@Int -> access(Param@1,x)) in neg(sum(map(f,Param@0)))
State: (List[6](Int[1,3])*List[6](Int[1,3]))	 Plan: Int
T: if (&&(==(size(Param@0),0),==(size(Param@1),0))) then collect(2,empty);if (&&(>(size(Param@0),0),>(size(Param@1),0))) then if (==(head(Param@0),head(Param@1))) then collect(1,'(0,'(tail(Param@0),tail(Param@1))));collect(1,'(1,'(tail(Param@0),tail(Param@1))));;if (>(size(Param@0),0)) then collect(1,'(2,'(tail(Param@0),Param@1)));;if (>(size(Param@1),0)) then collect(1,'(3,'(Param@0,tail(Param@1))));;if (&&(>(size(Param@0),1),>(size(Param@1),1))) then let a = head(Param@0) in let aa = head(tail(Param@0)) in let b = head(Param@1) in let bb = head(tail(Param@1)) in if (&&(==(a,bb),==(b,aa))) then collect(1,'(4,'(drop(Param@0,2),drop(Param@1,2))));;if (&&(>(size(Param@0),0),==(size(Param@1),0))) then collect(1,'(5,'(nil(0),nil(0))));
F:
	collect(0,+(Param@1,neg(access(Param@2,Param@0))))
	collect(0,0)



Step 4/5: Synthesize Eq Relation for States
Finished 3.281269
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)]



Step 5/5: Rewrite T via Eq Relation
Finished 12.278971
Cared Funtions
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)
State: (Int*Int)	 Plan: Int
T: if (&&(==(Param@0,0),==(Param@1,0))) then collect(2,empty);if (&&(>(Param@0,0),>(Param@1,0))) then if (==(access(Param@3.1,neg(Param@0)),access(Param@3.2,neg(Param@1)))) then collect(1,'(0,'(+(Param@0,neg(1)),+(Param@1,neg(1)))));collect(1,'(1,'(+(Param@0,neg(1)),+(Param@1,neg(1)))));;if (>(Param@0,0)) then collect(1,'(2,'(+(Param@0,neg(1)),Param@1)));;if (>(Param@1,0)) then collect(1,'(3,'(Param@0,+(Param@1,neg(1)))));;if (&&(>(Param@0,1),>(Param@1,1))) then if (&&(==(access(Param@3.1,neg(Param@0)),access(Param@3.2,+(1,neg(Param@1)))),==(access(Param@3.2,neg(Param@1)),access(Param@3.1,+(1,neg(Param@0)))))) then collect(1,'(4,'(+(Param@0,neg(2)),+(Param@1,neg(2)))));;if (&&(>(Param@0,0),==(Param@1,0))) then collect(1,'(5,'(0,0)));
F:
	collect(0,+(Param@1,neg(access(Param@2,Param@0))))
	collect(0,0)
