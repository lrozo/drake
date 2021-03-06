#include "constructPtrRigidBodyConstraint.h"
#include "RigidBodyManipulator.h"
#include "RigidBodyConstraint.h"
#include "drakeUtil.h"
using namespace Eigen;
using namespace std;


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if(!mxIsNumeric(prhs[0]))
  {
    mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","prhs[0] should be the constraint type");
  }
  int constraint_type = (int) mxGetScalar(prhs[0]);
  switch(constraint_type)
  {
    // QuasiStaticConstraint
    case RigidBodyConstraint::QuasiStaticConstraintType:
      {
        if(nrhs != 2 && nrhs!= 3 && nrhs != 4)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrPtrRigidBodyConstraintmex(RigidBodyConstraint::QuasiStaticConstraintType,robot_ptr,tspan,robotnum)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        int* robotnum;
        int num_robot;
        if(nrhs <= 3)
        {
          num_robot = 1;
          robotnum = new int[num_robot];
          robotnum[0] = 0;
        }
        else
        {
          num_robot = mxGetNumberOfElements(prhs[3]);
          double* robotnum_tmp = new double[num_robot];
          robotnum = new int[num_robot];
          memcpy(robotnum_tmp,mxGetPr(prhs[3]),sizeof(double)*num_robot);
          for(int i = 0;i<num_robot;i++)
          {
            robotnum[i] = (int) robotnum_tmp[i]-1;
          }
          delete[] robotnum_tmp;
        }
        if(nrhs<=2)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[2],tspan);
        }
        set<int> robotnumset(robotnum,robotnum+num_robot);
        QuasiStaticConstraint* cnst = new QuasiStaticConstraint(model,tspan,robotnumset);
        plhs[0] = createDrakeConstraintMexPointer((void*) cnst,"deleteRigidBodyConstraintmex","QuasiStaticConstraint");
        delete[] robotnum;
      }
      break;
    // PostureConstraint
    case RigidBodyConstraint::PostureConstraintType:
      {
        if(nrhs != 2 && nrhs != 3)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::PostureConstraintType,robot.mex_model_ptr,tspan)");
        }
        Vector2d tspan;
        if(nrhs == 2)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[2],tspan);
        }
        RigidBodyManipulator* robot = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        PostureConstraint* cnst = new PostureConstraint(robot,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","PostureConstraint");
      }
      break;
      // SingleTimeLinearPostureConstraint
    case RigidBodyConstraint::SingleTimeLinearPostureConstraintType:
      {
        if(nrhs != 7 && nrhs != 8)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::SingleTimeLinearPostureConstraintType,robot.mex_model_ptr,iAfun,jAvar,A,lb,ub,tspan");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs<=7)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[7],tspan);
        }
        if(!mxIsNumeric(prhs[2]) || !mxIsNumeric(prhs[3]) || !mxIsNumeric(prhs[4]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","iAfun, jAvar and A must be numeric");
        }
        int lenA = mxGetM(prhs[2]);
        if(mxGetM(prhs[3]) != lenA || mxGetM(prhs[4]) != lenA || mxGetN(prhs[2]) != 1 || mxGetN(prhs[3]) != 1 || mxGetN(prhs[4]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","iAfun,jAvar,A must be column vectors of the same size");
        }
        VectorXi iAfun(lenA);
        VectorXi jAvar(lenA);
        VectorXd A(lenA);
        for(int i = 0;i<lenA;i++)
        {
          iAfun(i) = (int) *(mxGetPr(prhs[2])+i)-1;
          jAvar(i) = (int) *(mxGetPr(prhs[3])+i)-1;
          A(i) = *(mxGetPr(prhs[4])+i);
        }
        if(!mxIsNumeric(prhs[5]) || !mxIsNumeric(prhs[6]) || mxGetM(prhs[5]) != mxGetM(prhs[6]) || mxGetN(prhs[5]) != 1 || mxGetN(prhs[6]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","lb and ub must be numeric column vectors of the same size");
        }
        int num_constraint = mxGetM(prhs[5]);
        VectorXd lb(num_constraint);
        VectorXd ub(num_constraint);
        memcpy(lb.data(),mxGetPr(prhs[5]),sizeof(double)*num_constraint);
        memcpy(ub.data(),mxGetPr(prhs[6]),sizeof(double)*num_constraint);
        SingleTimeLinearPostureConstraint* cnst = new SingleTimeLinearPostureConstraint(model,iAfun,jAvar,A,lb,ub,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","SingleTimeLinearPostureConstraint");
      }
      break;
      // AllBodiesClosestDistanceConstraint
    case RigidBodyConstraint::AllBodiesClosestDistanceConstraintType:
      {
        //DEBUG
        //cout << "nrhs = " << nrhs << endl;
        //END_DEBUG
        if(nrhs != 4 && nrhs != 5)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs",
              "Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::AllBodiesClosestDistanceConstraintType, robot.mex_model_ptr,lb,ub,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs == 4)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[4],tspan);
        }

        double lb = (double) mxGetScalar(prhs[2]);
        double ub = (double) mxGetScalar(prhs[3]);

        auto cnst = new AllBodiesClosestDistanceConstraint(model,lb,ub,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex",
                                        "AllBodiesClosestDistanceConstraint");
      }
      break;
      // WorldEulerConstraint
    case RigidBodyConstraint::WorldEulerConstraintType:
      {
        if(nrhs != 6 && nrhs != 5)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldEulerConstraintType,robot.mex_model_ptr,body,lb,ub,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldEulerConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs == 5)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[5],tspan);
        }
        if(!mxIsNumeric(prhs[2]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","body must be numeric");
        }
        int body = (int) mxGetScalar(prhs[2])-1;

        if(mxGetM(prhs[3]) != 3 || mxGetM(prhs[4]) != 3 || mxGetN(prhs[3]) != 1 || mxGetN(prhs[4]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","lb and ub should both be 3x1 double vectors");
        }
        Vector3d lb;
        Vector3d ub;
        memcpy(lb.data(),mxGetPr(prhs[3]),sizeof(double)*3);
        memcpy(ub.data(),mxGetPr(prhs[4]),sizeof(double)*3);
        cnst = new WorldEulerConstraint(model,body,lb,ub,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","WorldEulerConstraint");
      }
      break;
    // WorldGazeDirConstraint
    case RigidBodyConstraint::WorldGazeDirConstraintType:
      {
        if(nrhs != 7 && nrhs != 6)
        {  
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldGazeDirConstraintType,robot.mex_model_ptr,body,axis,dir,conethreshold,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldGazeDirConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs == 6)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[6],tspan);
        }
        if(!mxIsNumeric(prhs[2]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","body must be numeric");
        }
        int body = (int) mxGetScalar(prhs[2])-1;
        Vector3d axis;
        rigidBodyConstraintParse3dUnitVector(prhs[3], axis);
        Vector3d dir;
        rigidBodyConstraintParse3dUnitVector(prhs[4],dir);
        double conethreshold = rigidBodyConstraintParseGazeConethreshold(prhs[5]); 
        cnst = new WorldGazeDirConstraint(model,body,axis,dir,conethreshold,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst, "deleteRigidBodyConstraintmex","WorldGazeDirConstraint");
      }
      break;
    // WorldGazeOrientConstraint
    case RigidBodyConstraint::WorldGazeOrientConstraintType:
      {
        if(nrhs != 7 && nrhs != 8)
        {  
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldGazeOrientConstraintType, robot.mex_model_ptr,body,axis,quat_des,conethreshold,threshold,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldGazeOrientConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs == 7)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[7],tspan);
        }
        if(!mxIsNumeric(prhs[2]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","body must be numeric");
        }
        int body = (int) mxGetScalar(prhs[2])-1;
        Vector3d axis;
        rigidBodyConstraintParse3dUnitVector(prhs[3],axis);
        Vector4d quat_des;
        rigidBodyConstraintParseQuat(prhs[4],quat_des);
        double conethreshold = rigidBodyConstraintParseGazeConethreshold(prhs[5]);
        double threshold = rigidBodyConstraintParseGazeThreshold(prhs[6]);
        cnst = new WorldGazeOrientConstraint(model,body,axis,quat_des,conethreshold,threshold,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst, "deleteRigidBodyConstraintmex","WorldGazeOrientConstraint");
      }
      break;
    // WorldGazeTargetConstraint
    case RigidBodyConstraint::WorldGazeTargetConstraintType:
      {
        if(nrhs != 8 && nrhs != 7)
        {  
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldGazeTargetConstraintType,robot.mex_model_ptr,body,axis,target,gaze_origin,conethreshold,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldGazeTargetConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs == 7)
        {
          tspan<< -mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[7],tspan);
        }
        if(!mxIsNumeric(prhs[2]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","body must be numeric");
        }
        int body = (int) mxGetScalar(prhs[2])-1;
        Vector3d axis;
        rigidBodyConstraintParse3dUnitVector(prhs[3],axis);
        Vector3d target;
        assert(mxIsNumeric(prhs[4]));
        assert(mxGetM(prhs[4]) == 3 &&mxGetN(prhs[4]) == 1);
        memcpy(target.data(),mxGetPr(prhs[4]),sizeof(double)*3);
        Vector4d gaze_origin;
        Vector3d gaze_origin_tmp;
        assert(mxIsNumeric(prhs[5]));
        assert(mxGetM(prhs[5]) == 3 && mxGetN(prhs[5]) == 1);
        memcpy(gaze_origin_tmp.data(),mxGetPr(prhs[5]),sizeof(double)*3);
        gaze_origin.head(3) = gaze_origin_tmp;
        gaze_origin(3) = 1.0;
        double conethreshold = rigidBodyConstraintParseGazeConethreshold(prhs[6]);
        cnst = new WorldGazeTargetConstraint(model,body,axis,target,gaze_origin,conethreshold,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst, "deleteRigidBodyConstraintmex","WorldGazeTargetConstraint");
      }
      break;
    // RelativeGazeTargetConstraint
    case RigidBodyConstraint::RelativeGazeTargetConstraintType:
      {
        if(nrhs != 7 && nrhs != 8 && nrhs != 9)
        {  
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::RelativeGazeTargetConstraintType, robot.mex_model_ptr,bodyA,bodyB,axis,target,gaze_origin,conethreshold,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs < 9)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[8],tspan);
        }
        if(!mxIsNumeric(prhs[2]) || !mxIsNumeric(prhs[3]) || mxGetNumberOfElements(prhs[2]) != 1 || mxGetNumberOfElements(prhs[3]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","bodyA and bodyB should be numeric scalars");
        }
        int bodyA_idx = (int) mxGetScalar(prhs[2])-1;
        int bodyB_idx = (int) mxGetScalar(prhs[3])-1;
        if(!mxIsNumeric(prhs[4]) || mxGetM(prhs[4]) != 3 || mxGetN(prhs[4]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","axis should be 3x1 vector");
        }
        Vector3d axis;
        memcpy(axis.data(),mxGetPr(prhs[4]),sizeof(double)*3);
        double axis_norm = axis.norm();
        if(axis_norm<1e-10)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","axis should be a nonzero vector");
        }
        axis = axis/axis_norm;
        if(!mxIsNumeric(prhs[5]) || mxGetM(prhs[5]) != 3 || mxGetN(prhs[5]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","target should be 3x1 vector");
        }
        Vector3d target;
        memcpy(target.data(),mxGetPr(prhs[5]),sizeof(double)*3);
        if(!mxIsNumeric(prhs[6]) || mxGetM(prhs[6]) != 3 || mxGetN(prhs[6]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","gaze_origin should be 3x1 vector");
        }
        Vector4d gaze_origin;
        memcpy(gaze_origin.data(),mxGetPr(prhs[6]),sizeof(double)*3);
        gaze_origin(3) = 1.0;
        double conethreshold;
        if(nrhs<8)
        {
          conethreshold = 0.0;
        }
        else
        {
          if(!mxIsNumeric(prhs[7]) || mxGetNumberOfElements(prhs[7]) != 1)
          {
            if(mxGetNumberOfElements(prhs[7]) == 0)
            {
              conethreshold = 0.0;
            }
            else
            {
              mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","conethreshold should be a double scalar");
            }
          }
          conethreshold = mxGetScalar(prhs[7]);
          if(conethreshold<0)
          {
            mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","conethreshold should be nonnegative");
          }
        }
        RelativeGazeTargetConstraint* cnst = new RelativeGazeTargetConstraint(model,bodyA_idx, bodyB_idx,axis,target,gaze_origin,conethreshold,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst, "deleteRigidBodyConstraintmex","RelativeGazeTargetConstraint");
      }
      break;
    // RelativeGazeDirConstraint
    case RigidBodyConstraint::RelativeGazeDirConstraintType:
      {
        if(nrhs != 6 && nrhs != 7 && nrhs != 8)
        {  
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::RelativeGazeDirConstraintType, robot.mex_model_ptr,bodyA,bodyB,axis,dir,conethreshold,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs < 9)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[7],tspan);
        }
        if(!mxIsNumeric(prhs[2]) || !mxIsNumeric(prhs[3]) || mxGetNumberOfElements(prhs[2]) != 1 || mxGetNumberOfElements(prhs[3]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","bodyA and bodyB should be numeric scalars");
        }
        int bodyA_idx = (int) mxGetScalar(prhs[2])-1;
        int bodyB_idx = (int) mxGetScalar(prhs[3])-1;
        if(!mxIsNumeric(prhs[4]) || mxGetM(prhs[4]) != 3 || mxGetN(prhs[4]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","axis should be 3x1 vector");
        }
        Vector3d axis;
        memcpy(axis.data(),mxGetPr(prhs[4]),sizeof(double)*3);
        double axis_norm = axis.norm();
        if(axis_norm<1e-10)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","axis should be a nonzero vector");
        }
        axis = axis/axis_norm;
        if(!mxIsNumeric(prhs[5]) || mxGetM(prhs[5]) != 3 || mxGetN(prhs[5]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","dir should be 3x1 vector");
        }
        Vector3d dir;
        memcpy(dir.data(),mxGetPr(prhs[5]),sizeof(double)*3);
        double conethreshold;
        if(nrhs<7)
        {
          conethreshold = 0.0;
        }
        else
        {
          if(!mxIsNumeric(prhs[6]) || mxGetNumberOfElements(prhs[6]) != 1)
          {
            if(mxGetNumberOfElements(prhs[6]) == 0)
            {
              conethreshold = 0.0;
            }
            else
            {
              mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","conethreshold should be a double scalar");
            }
          }
          conethreshold = mxGetScalar(prhs[6]);
          if(conethreshold<0)
          {
            mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","conethreshold should be nonnegative");
          }
        }
        RelativeGazeDirConstraint* cnst = new RelativeGazeDirConstraint(model,bodyA_idx, bodyB_idx,axis,dir,conethreshold,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst, "deleteRigidBodyConstraintmex","RelativeGazeDirConstraint");
      }
      break;
      // WorldCoMConstraint
    case RigidBodyConstraint::WorldCoMConstraintType:
      {
        if(nrhs != 4 && nrhs != 5 && nrhs != 6)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldCoMConstraintType,robot.mex_model_ptr,lb,ub,tspan,robotnum)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldCoMConstraint* cnst = nullptr;
        Vector2d tspan;
        int* robotnum;
        int num_robot;
        if(nrhs <= 5)
        {
          num_robot = 1;
          robotnum = new int[num_robot];
          robotnum[0] = 0;
        }
        else
        {
          num_robot = mxGetNumberOfElements(prhs[5]);
          double* robotnum_tmp = new double[num_robot];
          robotnum = new int[num_robot];
          memcpy(robotnum_tmp,mxGetPr(prhs[5]),sizeof(double)*num_robot);
          for(int i = 0;i<num_robot;i++)
          {
            robotnum[i] = (int) robotnum_tmp[i]-1;
          }
          delete[] robotnum_tmp;
        }
        if(nrhs<=4)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[4],tspan);
        }
        set<int> robotnumset(robotnum,robotnum+num_robot);
        int n_pts = 1;
        if(mxGetM(prhs[2]) != 3 || mxGetM(prhs[3]) != 3 || mxGetN(prhs[2]) != n_pts || mxGetN(prhs[3]) != n_pts || !mxIsNumeric(prhs[2]) || !mxIsNumeric(prhs[3]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","lb and ub should both be 3x1 double vectors");
        }
        Vector3d lb;
        Vector3d ub;
        memcpy(lb.data(),mxGetPr(prhs[2]),sizeof(double)*3);
        memcpy(ub.data(),mxGetPr(prhs[3]),sizeof(double)*3);
        cnst = new WorldCoMConstraint(model,lb,ub,tspan,robotnumset);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","WorldCoMConstraint");
        delete[] robotnum;
      }
      break;
      // WorldPositionConstraint
    case RigidBodyConstraint::WorldPositionConstraintType:
      {
        if(nrhs!= 7 && nrhs != 6)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldPositionConstraintType, robot.mex_model_ptr,body,pts,lb,ub,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldPositionConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs <= 6)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[6],tspan);
        }
        if(!mxIsNumeric(prhs[2]) || mxGetM(prhs[2])!= 1 || mxGetN(prhs[2]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","body must be numeric");
        }
        int body = (int) mxGetScalar(prhs[2])-1;
        int n_pts = mxGetN(prhs[3]);
        if(!mxIsNumeric(prhs[3])||mxGetM(prhs[3]) != 3)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Argument 3 should be a double matrix with 3 rows");
        }
        MatrixXd pts_tmp(3,n_pts);
        memcpy(pts_tmp.data(),mxGetPr(prhs[3]),sizeof(double)*3*n_pts);
        MatrixXd pts(4,n_pts);
        pts.block(0,0,3,n_pts) = pts_tmp;
        pts.block(3,0,1,n_pts) = MatrixXd::Ones(1,n_pts);

        MatrixXd lb(3,n_pts);
        MatrixXd ub(3,n_pts);
        if(mxGetM(prhs[4]) != 3 || mxGetN(prhs[4]) != n_pts || mxGetM(prhs[5]) != 3 || mxGetN(prhs[5]) != n_pts)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","lb and ub should both be 3xn_pts double matrix");
        }
        memcpy(lb.data(),mxGetPr(prhs[4]),sizeof(double)*3*n_pts);
        memcpy(ub.data(),mxGetPr(prhs[5]),sizeof(double)*3*n_pts);
        cnst = new WorldPositionConstraint(model,body,pts,lb,ub,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","WorldPositionConstraint");
      }
      break;
    // WorldPositionInFrameConstraint
    case RigidBodyConstraint::WorldPositionInFrameConstraintType:
      {
        if(nrhs!= 8 && nrhs != 7)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldPositionInFrameConstraint, robot.mex_model_ptr,body,pts,lb,ub,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldPositionInFrameConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs <= 7)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[7],tspan);
        }
        int body = (int) mxGetScalar(prhs[2])-1;
        int n_pts = mxGetN(prhs[3]);
        if(!mxIsNumeric(prhs[3])||mxGetM(prhs[3]) != 3)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Argument 3 should be a double matrix with 3 rows");
        }
        MatrixXd pts_tmp(3,n_pts);
        memcpy(pts_tmp.data(),mxGetPr(prhs[3]),sizeof(double)*3*n_pts);
        MatrixXd pts(4,n_pts);
        pts.block(0,0,3,n_pts) = pts_tmp;
        pts.block(3,0,1,n_pts) = MatrixXd::Ones(1,n_pts);

        if(!mxIsNumeric(prhs[4]) || mxGetM(prhs[4]) != 4 || mxGetN(prhs[4]) != 4)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Argument 4 should be a 4x4 double matrix");
        }
        Matrix4d T_world_to_frame;
        memcpy(T_world_to_frame.data(),mxGetPr(prhs[4]),sizeof(double)*16);

        MatrixXd lb(3,n_pts);
        MatrixXd ub(3,n_pts);
        if(mxGetM(prhs[5]) != 3 || mxGetN(prhs[5]) != n_pts || mxGetM(prhs[6]) != 3 || mxGetN(prhs[6]) != n_pts)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","lb and ub should both be 3xn_pts double matrix");
        }
        memcpy(lb.data(),mxGetPr(prhs[5]),sizeof(double)*3*n_pts);
        memcpy(ub.data(),mxGetPr(prhs[6]),sizeof(double)*3*n_pts);

        cnst = new WorldPositionInFrameConstraint(model,body,pts,T_world_to_frame,
                                                  lb,ub,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","WorldPositionInFrameConstraint");
      }
      break;
      // WorldQuatConstraint
    case RigidBodyConstraint::WorldQuatConstraintType:
      {
        if(nrhs!= 6 && nrhs!= 5)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldQuatConstraintType,robot.mex_model_ptr,body,quat_des,tol,tspan)");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldQuatConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs <= 5)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[5],tspan);
        }
        int body = (int) mxGetScalar(prhs[2])-1;
        Vector4d quat_des;
        rigidBodyConstraintParseQuat(prhs[3],quat_des);
        double tol = mxGetScalar(prhs[4]);
        cnst = new WorldQuatConstraint(model,body,quat_des,tol,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","WorldQuatConstraint");
      }
      break;
      // Point2PointDistanceConstraint
    case RigidBodyConstraint::Point2PointDistanceConstraintType:
      {
        if(nrhs != 8 && nrhs != 9)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructRigidBodyConstraintmex(RigidBodyConstraint::Point2PointDistanceConstraintType, robot.mex_model_ptr,bodyA,bodyB,ptA,ptB,dist_lb,dist_ub,tspan");
        }
        RigidBodyManipulator *model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs <= 8)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[8],tspan);
        }
        if(!mxIsNumeric(prhs[2]) || !mxIsNumeric(prhs[3]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","bodyA and bodyB must be numeric");
        }
        if(mxGetNumberOfElements(prhs[2]) != 1 || mxGetNumberOfElements(prhs[3]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","bodyA and bodyB must be a scalar");
        }
        int bodyA = (int) mxGetScalar(prhs[2])-1;
        int bodyB = (int) mxGetScalar(prhs[3])-1;
        if(bodyA>=model->num_bodies || bodyA < -1 || bodyB>= model->num_bodies || bodyB < -1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","bodyA and bodyB must be within [0 robot.getNumBodies]");
        }
        if(bodyA == bodyB)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","bodyA and bodyB should be different");
        }
        if(!mxIsNumeric(prhs[4]) || !mxIsNumeric(prhs[5]))
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","ptA and ptB should be numeric");
        }
        int npts = mxGetN(prhs[4]);
        if(mxGetM(prhs[4]) != 3 || mxGetM(prhs[5]) != 3)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","ptA and ptB should have 3 rows");
        }
        if(mxGetN(prhs[5]) != npts)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","ptA and ptB should have the same number of columns");
        }
        MatrixXd ptA_tmp(3,npts);
        MatrixXd ptB_tmp(3,npts);
        memcpy(ptA_tmp.data(),mxGetPr(prhs[4]),sizeof(double)*3*npts);
        memcpy(ptB_tmp.data(),mxGetPr(prhs[5]),sizeof(double)*3*npts);
        MatrixXd ptA(4,npts);
        MatrixXd ptB(4,npts);
        ptA.block(0,0,3,npts) = ptA_tmp;
        ptB.block(0,0,3,npts) = ptB_tmp;
        ptA.row(3) = MatrixXd::Ones(1,npts);
        ptB.row(3) = MatrixXd::Ones(1,npts);
        if(!mxIsNumeric(prhs[6]) || !mxIsNumeric(prhs[7]) || mxGetM(prhs[6]) != 1 || mxGetM(prhs[7]) != 1 || mxGetN(prhs[6]) != npts || mxGetN(prhs[7]) != npts)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","dist_lb and dist_ub must be 1 x npts numeric");
        }
        VectorXd dist_lb(npts);
        VectorXd dist_ub(npts);
        memcpy(dist_lb.data(),mxGetPr(prhs[6]),sizeof(double)*npts);
        memcpy(dist_ub.data(),mxGetPr(prhs[7]),sizeof(double)*npts);
        for(int i = 0;i<npts;i++)
        {
          if(dist_lb(i)>dist_ub(i))
          {
            mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","dist_lb must be no larger than dist_ub");
          }
        }
        Point2PointDistanceConstraint* cnst = new Point2PointDistanceConstraint(model,bodyA,bodyB,ptA,ptB,dist_lb,dist_ub,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","Point2PointDistanceConstraint");
      }
      break;
    // Point2LineSegDistConstraint
    case RigidBodyConstraint::Point2LineSegDistConstraintType:
      {
        if(nrhs != 8 && nrhs != 9)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::Point2LineSegDistConstraint, robot.mex_model_ptr,pt_body,pt,line_body,line_ends,lb,ub,tspan");
        }
        RigidBodyManipulator *model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs <= 8)
        {
          tspan<< -mxGetInf(), mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[8],tspan);
        }
        if(!mxIsNumeric(prhs[2])||mxGetNumberOfElements(prhs[2]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","pt_body should be a numeric scalar");
        }
        int pt_body = (int) mxGetScalar(prhs[2])-1;
        if(pt_body>=model->num_bodies || pt_body<0)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrPoint2LineSegDistConstraintmex:BadInputs","pt_body is invalid");
        }
        if(!mxIsNumeric(prhs[3]) || mxGetM(prhs[3]) != 3 || mxGetN(prhs[3]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","pt should be a 3x1 vector");
        }
        Vector4d pt;
        memcpy(pt.data(),mxGetPr(prhs[3]),sizeof(double)*3);
        pt(3) = 1.0;
        if(!mxIsNumeric(prhs[4])||mxGetNumberOfElements(prhs[4]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","line_body should be a numeric scalar");
        }
        int line_body = (int) mxGetScalar(prhs[4])-1;
        if(line_body>=model->num_bodies || line_body<0)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","line_body is invalid");
        }
        if(!mxIsNumeric(prhs[5]) || mxGetM(prhs[5]) != 3 || mxGetN(prhs[5]) != 2)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","line_ends should be a 3x2 vector");
        }
        Matrix<double,4,2> line_ends;
        Matrix<double,3,2> line_ends_tmp;
        memcpy(line_ends_tmp.data(),mxGetPr(prhs[5]),sizeof(double)*6);
        line_ends.block(0,0,3,2) = line_ends_tmp;
        line_ends.row(3) = MatrixXd::Ones(1,2);
        if(!mxIsNumeric(prhs[6]) || !mxIsNumeric(prhs[7]) || mxGetNumberOfElements(prhs[6]) != 1 || mxGetNumberOfElements(prhs[7]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","dist_lb, dist_ub should be scalars");
        }
        double dist_lb = mxGetScalar(prhs[6]);
        double dist_ub = mxGetScalar(prhs[7]);
        if(dist_lb<0 || dist_lb>dist_ub)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","dist_lb should be nonnegative, and dist_ub should be no less than dist_lb");
        }
        Point2LineSegDistConstraint* cnst = new Point2LineSegDistConstraint(model,pt_body,pt,line_body,line_ends,dist_lb,dist_ub,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*) cnst,"deleteRigidBodyConstraintmex","Point2LineSegDistConstraint");
      }
      break;
    // WorldFixedPositionConstraint
    case RigidBodyConstraint::WorldFixedPositionConstraintType:
      {
        if(nrhs != 5 && nrhs != 4)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs", "Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldFixedPositionConstraintType, robot.mex_model_ptr,body,pts,tspan)");
        }
        RigidBodyManipulator *model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldFixedPositionConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs <= 4)
        {
          tspan<< -mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[4],tspan);
        }
        int body = (int) mxGetScalar(prhs[2])-1;
        int n_pts = mxGetN(prhs[3]);
        if(!mxIsNumeric(prhs[3])||mxGetM(prhs[3]) != 3)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Argument 4 should be a double matrix with 3 rows");
        }
        MatrixXd pts_tmp(3,n_pts);
        memcpy(pts_tmp.data(),mxGetPr(prhs[3]),sizeof(double)*3*n_pts);
        MatrixXd pts(4,n_pts);
        pts.block(0,0,3,n_pts) = pts_tmp;
        pts.block(3,0,1,n_pts) = MatrixXd::Ones(1,n_pts);

        cnst = new WorldFixedPositionConstraint(model,body,pts,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","WorldFixedPositionConstraint");
      }
      break;
    // WorldFixedOrientConstraint
    case RigidBodyConstraint::WorldFixedOrientConstraintType:
      {
        if(nrhs != 4 && nrhs != 3)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs", "Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldFixedOrientConstraintType,robot.mex_model_ptr,body,tspan)");
        }
        RigidBodyManipulator *model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldFixedOrientConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs <= 3)
        {
          tspan<< -mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[3],tspan);
        }
        int body = (int) mxGetScalar(prhs[2])-1;

        cnst = new WorldFixedOrientConstraint(model,body,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","WorldFixedOrientConstraint");
      }
      break;
    // WorldFixedBodyPoseConstraint
    case RigidBodyConstraint::WorldFixedBodyPoseConstraintType:
      {
        if(nrhs != 4 && nrhs != 3)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs", "Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::WorldFixedBodyPoseConstraintType,robot.mex_model_ptr,body,tspan)");
        }
        RigidBodyManipulator *model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        WorldFixedBodyPoseConstraint* cnst = nullptr;
        Vector2d tspan;
        if(nrhs <= 3)
        {
          tspan<< -mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[3],tspan);
        }
        int body = (int) mxGetScalar(prhs[2])-1;

        cnst = new WorldFixedBodyPoseConstraint(model,body,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","WorldFixedBodyPoseConstraint");
      }
      break;
      // PostureChangeConstraintType
    case RigidBodyConstraint::PostureChangeConstraintType:
      {
        if(nrhs != 6 && nrhs != 5)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::PostureChangeConstraintType,robot.mex_model_ptr,joint_ind,lb_change,ub_change,tspan");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs <= 5)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[5],tspan);
        }
        if(!mxIsNumeric(prhs[2]) || mxGetN(prhs[2]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","joint_ind must be a column numeric vector");
        }
        int num_joints = mxGetM(prhs[2]);
        VectorXd joint_ind_tmp(num_joints);
        memcpy(joint_ind_tmp.data(),mxGetPr(prhs[2]),sizeof(double)*num_joints);
        VectorXi joint_ind(num_joints);
        for(int i = 0;i<num_joints;i++)
        {
          joint_ind(i) = (int) joint_ind_tmp(i)-1;
          if(joint_ind(i)<0 || joint_ind(i)>=model->num_dof)
          {
            mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","joint_ind must be within [1,nq]");
          }
        }
        if(!mxIsNumeric(prhs[3]) || mxGetM(prhs[3]) != num_joints || mxGetN(prhs[3]) != 1 ||!mxIsNumeric(prhs[4]) || mxGetM(prhs[4]) != num_joints || mxGetN(prhs[4]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","lb_change and upper bound change must be both numeric vector, with the same size as joint_ind");
        }
        VectorXd lb_change(num_joints);
        VectorXd ub_change(num_joints);
        memcpy(lb_change.data(),mxGetPr(prhs[3]),sizeof(double)*num_joints);
        memcpy(ub_change.data(),mxGetPr(prhs[4]),sizeof(double)*num_joints);
        for(int i = 0;i<num_joints;i++)
        {
          double lb_change_min = model->joint_limit_min[joint_ind(i)]-model->joint_limit_max[joint_ind(i)];
          double ub_change_max = model->joint_limit_max[joint_ind(i)]-model->joint_limit_min[joint_ind(i)];
          lb_change(i) = (lb_change_min<lb_change(i)?lb_change(i):lb_change_min);
          ub_change(i) = (ub_change_max>ub_change(i)?ub_change(i):ub_change_max);
          if(lb_change(i)>ub_change(i))
          {
            mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","lb_change must be no larger than ub_change");
          }
        }
        PostureChangeConstraint* cnst = new PostureChangeConstraint(model,joint_ind,lb_change,ub_change,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","PostureChangeConstraint");
      }
      break;
    case RigidBodyConstraint::RelativePositionConstraintType:
      {
        if(nrhs != 9 && nrhs != 8)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::RelativePositionConstraintType,robot.mex_model_ptr,pts,lb,ub,bodyA_idx,bodyB_idx,bTbp,tspan");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs <= 8)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[8],tspan);
        }
        if(!mxIsNumeric(prhs[2])|| mxGetM(prhs[2]) != 3)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Argument 3 (pts) should be a double matrix with 3 rows");
        }
        int n_pts = mxGetN(prhs[3]);
        MatrixXd pts_tmp(3,n_pts);
        memcpy(pts_tmp.data(),mxGetPr(prhs[2]),sizeof(double)*3*n_pts);
        MatrixXd pts(4,n_pts);
        pts.block(0,0,3,n_pts) = pts_tmp;
        pts.block(3,0,1,n_pts) = MatrixXd::Ones(1,n_pts);

        MatrixXd lb(3,n_pts);
        MatrixXd ub(3,n_pts);
        if(!mxIsNumeric(prhs[3]) || mxGetM(prhs[3]) != 3 || mxGetN(prhs[3]) != n_pts || !mxIsNumeric(prhs[4]) || mxGetM(prhs[4]) != 3 || mxGetN(prhs[4]) != n_pts)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","lb and ub should both be 3xn_pts double matrix");
        }
        memcpy(lb.data(),mxGetPr(prhs[3]),sizeof(double)*3*n_pts);
        memcpy(ub.data(),mxGetPr(prhs[4]),sizeof(double)*3*n_pts);
        
        if(!mxIsNumeric(prhs[5]) || mxGetM(prhs[5]) != 1 || mxGetN(prhs[5]) != 1 || !mxIsNumeric(prhs[6]) || mxGetM(prhs[6]) != 1 || mxGetN(prhs[6]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","bodyA_idx and bodyB_idx should be numeric scalars");
        }
        int bodyA_idx = static_cast<int>(mxGetScalar(prhs[5])-1);
        int bodyB_idx = static_cast<int>(mxGetScalar(prhs[6])-1);

        Matrix<double,7,1> bTbp;
        if(!mxIsNumeric(prhs[7]) || mxGetM(prhs[7]) != 7 || mxGetN(prhs[7]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","bTbp should be 7x1 numeric vector");
        }
        memcpy(bTbp.data(),mxGetPr(prhs[7]),sizeof(double)*7);
        double quat_norm = bTbp.block(3,0,4,1).norm();
        if(abs(quat_norm-1)>1e-5)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","bTbp(4:7) should be a unit quaternion");
        }
        bTbp.block(3,0,4,1) /= quat_norm;
        RelativePositionConstraint* cnst = new RelativePositionConstraint(model,pts,lb,ub,bodyA_idx,bodyB_idx,bTbp,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","RelativePositionConstraint");
      }
      break;
    case RigidBodyConstraint::RelativeQuatConstraintType:
      {
        if(nrhs != 7 && nrhs != 6)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Usage ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint::RelativeQuatConstraintType,robot.mex_model_ptr,bodyA_idx,bodyB_idx,quat_des,tol,tspan");
        }
        RigidBodyManipulator* model = (RigidBodyManipulator*) getDrakeMexPointer(prhs[1]);
        Vector2d tspan;
        if(nrhs <= 6)
        {
          tspan<<-mxGetInf(),mxGetInf();
        }
        else
        {
          rigidBodyConstraintParseTspan(prhs[6],tspan);
        }
        if(!mxIsNumeric(prhs[2])|| mxGetM(prhs[2]) != 1 || mxGetN(prhs[2]) != 1 || !mxIsNumeric(prhs[3])|| mxGetM(prhs[3]) != 1 || mxGetN(prhs[3]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Argument 3 (bodyA_idx) and 4 (bodyB_idx) should be both scalars");
        }
        int bodyA_idx = static_cast<int>(mxGetScalar(prhs[2]))-1;
        int bodyB_idx = static_cast<int>(mxGetScalar(prhs[3]))-1;
        if(!mxIsNumeric(prhs[4]) || mxGetM(prhs[4]) != 4 || mxGetN(prhs[4]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Argument 5 (quat_des) should be 4 x 1 double vector");
        }
        Vector4d quat_des;
        memcpy(quat_des.data(),mxGetPr(prhs[4]),sizeof(double)*4);
        double quat_norm = quat_des.norm();
        if(abs(quat_norm-1.0)>1e-5)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Argument 5 should be a quaternion with unit length");
        }
        if(!mxIsNumeric(prhs[5]) || mxGetM(prhs[5]) != 1 || mxGetN(prhs[5]) != 1)
        {
          mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Argument 6 (tol) should be a double scalar");
        }
        double tol = mxGetScalar(prhs[5]);
        RelativeQuatConstraint* cnst = new RelativeQuatConstraint(model,bodyA_idx,bodyB_idx,quat_des,tol,tspan);
        plhs[0] = createDrakeConstraintMexPointer((void*)cnst,"deleteRigidBodyConstraintmex","RelativeQuatConstraint");
      }
      break;
    default:
      mexErrMsgIdAndTxt("Drake:constructPtrRigidBodyConstraintmex:BadInputs","Unsupported constraint type");
      break;

  }
}
