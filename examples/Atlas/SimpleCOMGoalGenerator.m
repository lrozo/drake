classdef SimpleCOMGoalGenerator < DrakeSystem
  % simple system for generating COM goals at the center of the 
  % support foot polygon for Atlas (minimal contact model)
   
  methods
    function obj = SimpleCOMGoalGenerator(r)
      typecheck(r,'TimeSteppingRigidBodyManipulator');
      
      comframe = AtlasCOM(r);

      obj = obj@DrakeSystem(0,comframe.dim,r.getNumStates,comframe.dim,false,true);
      obj = setSampleTime(obj,[.01;0]); % update at 100 Hz
      obj = setInputFrame(obj,r.getStateFrame);
      obj = setOutputFrame(obj,comframe);

      obj.manip = r;
    end
    
    function com_des = getInitialState(obj)
      com_des = zeros(3,1);
    end
        
    function com_des = update(obj,t,com_des,x)
      nq = obj.manip.getNumStates()/2;
      q = x(1:nq);
      
      gc = obj.manip.contactPositions(q);
      
      % compute desired COM projection
      % assumes minimal contact model for now
      k = convhull(gc(1:2,:)');
      com_des = [mean(gc(1:2,k),2);0.95];
    end
    
    function y = output(obj,t,com_des,x)
      y = com_des;
    end
  end
  
  properties
    manip
  end
end