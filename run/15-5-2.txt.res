Step 1/5: Init Sampler
Finished
State type: (List[5](Int[1,3])*List[6](Int[1,3]))
Env type: [List[6](Int[1,3])]



Step 2/5: Synthesize PreOrder for Plan
Finished 0.117756
Cmp:[<=:let f = (\ x@Int -> access(Param@1,x)) in neg(sum(map(f,Param@0)))]



Step 3/5: Rewrite F via PreOrders
Finished 23.158561
Cared Funtions
	let f = (\ x@Int -> access(Param@1,x)) in neg(sum(map(f,Param@0)))
State: (List[6](Int[1,3])*List[6](Int[1,3]))	 Plan: Int
T: if (&&(==(size(Param@0),0),==(size(Param@1),0))) then collect(4,empty);if (&&(>(size(Param@0),0),>(size(Param@1),0))) then collect(2,'(0,head(Param@0),head(Param@1),'(tail(Param@0),tail(Param@1))));collect(1,'(1,'(tail(Param@0),tail(Param@1))));;if (>(size(Param@0),0)) then collect(1,'(2,'(tail(Param@0),Param@1)));;if (>(size(Param@1),0)) then collect(1,'(3,'(Param@0,tail(Param@1))));;if (&&(>(size(Param@0),1),>(size(Param@1),1))) then let a = head(Param@0) in let aa = head(tail(Param@0)) in let b = head(Param@1) in let bb = head(tail(Param@1)) in if (&&(==(a,bb),==(b,aa))) then collect(3,'(4,'(a,aa),'(bb,b),'(drop(Param@0,2),drop(Param@1,2))));;if (&&(>(size(Param@0),0),==(size(Param@1),0))) then collect(1,'(5,'(nil(0),nil(0))));
F:
	collect(0,+(Param@1,neg(access(Param@2,Param@0))))
	if (==(Param@1,Param@2)) then collect(0,+(Param@3,neg(access(Param@4,0))))
	if (&&(==(Param@1.1,Param@2.1),==(Param@1.2,Param@2.2))) then collect(0,+(Param@3,neg(access(Param@4,Param@0))))
	collect(0,0)



Step 4/5: Synthesize Eq Relation for States
Finished 1.899876
Eq:[fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1),fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)]



Step 5/5: Rewrite T via Eq Relation
Finished 16.160080
Cared Funtions
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@1)
	fold((\ tmp0@Int tmp1@Int -> +(1,tmp1)),0,Param@0)
State: (Int*Int)	 Plan: Int
T: if (&&(==(Param@1,0),==(Param@0,0))) then collect(4,empty);if (&&(>(Param@1,0),>(Param@0,0))) then collect(2,'(0,access(Param@3.1,neg(Param@1)),access(Param@3.2,neg(Param@0)),'(+(Param@0,neg(1)),+(Param@1,neg(1)))));collect(1,'(1,'(+(Param@0,neg(1)),+(Param@1,neg(1)))));;if (>(Param@1,0)) then collect(1,'(2,'(Param@0,+(Param@1,neg(1)))));;if (>(Param@0,0)) then collect(1,'(3,'(+(Param@0,neg(1)),Param@1)));;if (&&(>(Param@1,1),>(Param@0,1))) then if (&&(==(access(Param@3.1,neg(Param@1)),access(Param@3.2,+(1,neg(Param@0)))),==(access(Param@3.2,neg(Param@0)),access(Param@3.1,+(1,neg(Param@1)))))) then collect(3,'(4,'(access(Param@3.1,neg(Param@1)),access(Param@3.2,neg(Param@0))),'(access(Param@3.1,neg(Param@1)),access(Param@3.2,neg(Param@0))),'(+(Param@0,neg(2)),+(Param@1,neg(2)))));;if (&&(>(Param@1,0),==(Param@0,0))) then collect(1,'(5,'(0,0)));
F:
	collect(0,+(Param@1,neg(access(Param@2,Param@0))))
	if (==(Param@1,Param@2)) then collect(0,+(Param@3,neg(access(Param@4,0))))
	if (&&(==(Param@1.1,Param@2.1),==(Param@1.2,Param@2.2))) then collect(0,+(Param@3,neg(access(Param@4,Param@0))))
	collect(0,0)
