classdef RigidBodyGeometry

  methods % to be implemented in derived classes
    pts = getPoints(obj);             % returned in body coordinates
    pts = getBoundingBoxPoints(obj);  % returned in body coordinates
    lcmt_viewer_geometry_data = serializeToLCM(obj);
  end
  
  methods
    function obj = RigidBodyGeometry(bullet_shape_id,varargin)
      % obj = RigidBodyGeometry(bullet_shape_id) constructs a
      % RigidBodyGeometry object with the geometry-to-body transform set to
      % identity.
      %
      % obj = RigidBodyGeometry(bullet_shape_id,T) constructs a
      % RigidBodyGeometry object with the geometry-to-body transform T.
      % 
      % obj = RigidBodyGeometry(bullet_shape_id,xyz,rpy) constructs a
      % RigidBodyGeometry object with the geometry-to-body transform specified
      % by the position, xyz, and Euler angles, rpy.
      %
      % @param bullet_shape_id - Integer that tells the DrakeCollision library
      % what type of geometry this is.
      % @param T - 4x4 homogenous transform from geometry-frame to body-frame
      % @param xyz - 3-element vector specifying the position of the geometry
      % in the body-frame
      % @param rpy - 3-element vector of Euler angles specifying the orientation of the
      % geometry in the body-frame
      if nargin < 2
        T_geometry_to_body = eye(4);
      elseif nargin < 3
        typecheck(varargin{1},'numeric');
        sizecheck(varargin{1},[4, 4]);
        T_geometry_to_body = varargin{1};
      else
        typecheck(varargin{1},'numeric');
        typecheck(varargin{2},'numeric');
        sizecheck(varargin{1},3);
        sizecheck(varargin{2},3);
        xyz = reshape(varargin{1},3,1);
        rpy = reshape(varargin{2},3,1);
        T_geometry_to_body = [rpy2rotmat(rpy), xyz; zeros(1,3),1];
      end
      obj.bullet_shape_id = bullet_shape_id;
      obj.T = T_geometry_to_body;
    end
    
    function [x,y,z,c] = getPatchData(obj,x_axis,y_axis,view_axis)
      % output is compatible with patch for 2D viewing
      % still returns a 3xn, but the z-axis is constant (just meant for
      % depth ordering in the 2d figure)
      
      Rview = [x_axis, y_axis, view_axis]';
      valuecheck(svd(Rview),[1;1;1]);  % assert that it's orthonormal
      
      pts = getPoints(obj);
      
      % project it into 2D frame
      pts = Rview*pts;
      
      % keep only convex hull (todo: do better here)
      ind = convhull(pts(1,:),pts(2,:));
      z = max(pts(3,:));
      
      % take it back out of view coordinates
      pts = Rview'*[pts(1:2,ind); repmat(z,1,length(ind))];
      x = pts(1,:)';
      y = pts(2,:)';
      z = pts(3,:)';
      c = obj.c;
    end

    function pts = getTerrainContactPoints(obj)
      % pts = getTerrainContactPoints(obj)
      %
      % @param  obj - RigidBodyGeometry object
      % @retval pts - 3xm array of points on this geometry (in link frame) that
      %               can collide with the world.
      pts = [];
    end
    
  end
  
  methods (Static)
    function obj = parseURDFNode(node,x0,rpy,model,robotnum,options)
      T= [rpy2rotmat(rpy),x0; 0 0 0 1];
      
      childNodes = node.getChildNodes();
      for i=1:childNodes.getLength()
        thisNode = childNodes.item(i-1);
        switch (lower(char(thisNode.getNodeName())))
          case 'box'
            size = parseParamString(model,robotnum,char(thisNode.getAttribute('size')));
            obj = RigidBodyBox(size);
          case 'sphere'
            r = parseParamString(model,robotnum,char(thisNode.getAttribute('radius')));
            obj = RigidBodySphere(r);
          case 'cylinder'
            r = parseParamString(model,robotnum,char(thisNode.getAttribute('radius')));
            l = parseParamString(model,robotnum,char(thisNode.getAttribute('length')));
            obj = RigidBodyCylinder(r,l);  % l/2
          case 'mesh'
            filename=char(thisNode.getAttribute('filename'));
            scale = 1;
            if thisNode.hasAttribute('scale')
              scale = parseParamString(model,robotnum,char(thisNode.getAttribute('scale')));
            end
            
            if ~isempty(strfind(filename,'package://'))
              filename=strrep(filename,'package://','');
              [package,filename]=strtok(filename,filesep);
              filename=[rospack(package),filename];
            else
              [path,name,ext] = fileparts(filename);
              if (isempty(path) || path(1)~=filesep)  % the it's a relative path
                path = fullfile(options.urdfpath,path);
              end
              filename = fullfile(path,[name,ext]);
            end
            
            obj = RigidBodyMesh(GetFullPath(filename));
            obj.scale = scale;
          case 'capsule'
            r = parseParamString(model,robotnum,char(thisNode.getAttribute('radius')));
            l = parseParamString(model,robotnum,char(thisNode.getAttribute('length')));
            obj = RigidBodyCapsule(r,l);  % l/2
        end
        obj.T = T;
      end
    end

  end
  
  properties  % note: constructModelmex currently depends on these being public
    T = eye(4);  % coordinate transform (from geometry to link coordinates)
    c = [.7 .7 .7];  % 3x1 color
    bullet_shape_id = 0;  % UNKNOWN
  end
end
