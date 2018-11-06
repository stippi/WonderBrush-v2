Polynomial.TOLERANCE=1e-6;
Polynomial.ACCURACY=6;

function Polynomial()
{
	this.init(arguments);
}

Polynomial.prototype.init=function(coefs)
{
	this.coefs=new Array();
	for(var i=coefs.length-1;i>=0;i--)
		this.coefs.push(coefs[i]);
}

Polynomial.prototype.eval=function(x)
{
	var result=0;
	for(var i=this.coefs.length-1;i>=0;i--)
	result=result*x+this.coefs[i];
	return result;
}

Polynomial.prototype.multiply=function(that)
{
	var result=new Polynomial();
	for(var i=0;i<=this.getDegree()+that.getDegree();i++)
		result.coefs.push(0);
	for(var i=0;i<=this.getDegree();i++)
		for(var j=0;j<=that.getDegree();j++)
			result.coefs[i+j]+=this.coefs[i]*that.coefs[j];
	return result;
}

Polynomial.prototype.divide_scalar=function(scalar)
{
	for(var i=0;i<this.coefs.length;i++)
		this.coefs[i]/=scalar;
}

Polynomial.prototype.simplify=function()
{
	for(var i=this.getDegree();i>=0;i--) {
		if(Math.abs(this.coefs[i])<=Polynomial.TOLERANCE)
			this.coefs.pop();
		else break;
	}
}

Polynomial.prototype.bisection=function(min,max)
{
	var minValue=this.eval(min);
	var maxValue=this.eval(max);
	var result;

	if(Math.abs(minValue)<=Polynomial.TOLERANCE)
		result=min;
	else if(Math.abs(maxValue)<=Polynomial.TOLERANCE)
		result=max;
	else if(minValue*maxValue<=0) {
		var tmp1=Math.log(max-min);
		var tmp2=Math.log(10)*Polynomial.ACCURACY;
		var iters=Math.ceil((tmp1+tmp2)/Math.log(2));

		for(var i=0;i<iters;i++) {
			result=0.5*(min+max);
			var value=this.eval(result);

			if(Math.abs(value)<=Polynomial.TOLERANCE) {
				break;
			}
			if(value*minValue<0){
				max=result;
				maxValue=value;
			} else {
				min=result;
				minValue=value;
			}
		}
	}
	return result;
}
Polynomial.prototype.toString=function()
{
	var coefs=new Array();
	var signs=new Array();

	for(var i=this.coefs.length-1;i>=0;i--) {
		var value=this.coefs[i];

		if(value!=0) {
			var sign=(value<0)?" - ":" + ";
			value=Math.abs(value);
			if(i>0)
				if(value==1)
					value="x";
				else value+="x";
			if(i>1)
				value+="^"+i;
			signs.push(sign);
			coefs.push(value);
		}
	}

	signs[0]=(signs[0]==" + ")?"":"-";
	var result="";

	for(var i=0;i<coefs.length;i++)
		result+=signs[i]+coefs[i];

	return result;
}

Polynomial.prototype.getDegree=function()
{
	return this.coefs.length-1;
}

Polynomial.prototype.getDerivative=function()
{
	var derivative=new Polynomial();

	for(var i=1;i<this.coefs.length;i++){
		derivative.coefs.push(i*this.coefs[i]);
	}

	return derivative;
}

Polynomial.prototype.getRoots=function()
{
	var result;
	this.simplify();

	switch (this.getDegree()) {
		case 0:
			result=new Array();
			break;
		case 1:
			result=this.getLinearRoot();
			break;
		case 2:
			result=this.getQuadraticRoots();
			break;
		case 3:
			result=this.getCubicRoots();
			break;
		case 4:
			result=this.getQuarticRoots();
			break;
		default:
			result=new Array();
	}
	return result;
}

Polynomial.prototype.getRootsInInterval=function(min,max)
{
	var roots=new Array();
	var root;
	if (this.getDegree()==1) {
		root=this.bisection(min,max);
		if (root!=null)
			roots.push(root);
	} else {
		var deriv=this.getDerivative();
		var droots=deriv.getRootsInInterval(min,max);

		if (droots.length>0) {
			root=this.bisection(min,droots[0]);
			if (root!=null)
				roots.push(root);

			for (i=0;i<=droots.length-2;i++) {
				root=this.bisection(droots[i],droots[i+1]);
				if (root!=null)
					roots.push(root);
			}
			root=this.bisection(droots[droots.length-1],max);

			if (root!=null)
				roots.push(root);
		} else {
			root=this.bisection(min,max);

			if (root!=null)
				roots.push(root);
		}
	}
	return roots;
}

Polynomial.prototype.getLinearRoot=function()
{
	var result=new Array();
	var a=this.coefs[1];

	if (a!=0)
		result.push(-this.coefs[0]/a);

	return result;
}

Polynomial.prototype.getQuadraticRoots=function()
{
	var results=new Array();

	if (this.getDegree()==2) {
		var a=this.coefs[2];
		var b=this.coefs[1]/a;
		var c=this.coefs[0]/a;
		var d=b*b-4*c;

		if (d>0) {
			var e=Math.sqrt(d);
			results.push(0.5*(-b+e));
			results.push(0.5*(-b-e));
		} else if(d==0) {
			results.push(0.5*-b);
		}
	}
	return results;
}

Polynomial.prototype.getCubicRoots=function()
{
	var results=new Array();
	if (this.getDegree()==3) {
		var c3=this.coefs[3];
		var c2=this.coefs[2]/c3;
		var c1=this.coefs[1]/c3;
		var c0=this.coefs[0]/c3;
		var a=(3*c1-c2*c2)/3;
		var b=(2*c2*c2*c2-9*c1*c2+27*c0)/27;
		var offset=c2/3;
		var discrim=b*b/4 + a*a*a/27;
		var halfB=b/2;

		if (Math.abs(discrim)<=Polynomial.TOLERANCE)
			disrim=0;
		if (discrim>0) {
			var e=Math.sqrt(discrim);
			var tmp;
			var root;
			tmp=-halfB+e;
			if (tmp>=0)
				root=Math.pow(tmp,1/3);
			else
				root=-Math.pow(-tmp,1/3);
			tmp=-halfB-e;
			if (tmp>=0)
				root+=Math.pow(tmp,1/3);
			else
				root-=Math.pow(-tmp,1/3);
			results.push(root-offset);
		} else if (discrim<0) {
			var distance=Math.sqrt(-a/3);
			var angle=Math.atan2(Math.sqrt(-discrim),-halfB)/3;
			var cos=Math.cos(angle);
			var sin=Math.sin(angle);
			var sqrt3=Math.sqrt(3);
			results.push(2*distance*cos-offset);
			results.push(-distance*(cos+sqrt3*sin)-offset);
			results.push(-distance*(cos-sqrt3*sin)-offset);
		} else {
			var tmp;
			if (halfB>=0)
				tmp=-Math.pow(halfB,1/3);
			else
				tmp=Math.pow(-halfB,1/3);
			results.push(2*tmp-offset);
			results.push(-tmp-offset);
		}
	}
	return results;
}

Polynomial.prototype.getQuarticRoots=function()
{
	var results=new Array();
	if (this.getDegree()==4) {
		var c4=this.coefs[4];
		var c3=this.coefs[3]/c4;
		var c2=this.coefs[2]/c4;
		var c1=this.coefs[1]/c4;
		var c0=this.coefs[0]/c4;

		var resolveRoots=new Polynomial(1,-c2,c3*c1-4*c0,-c3*c3*c0+4*c2*c0-c1*c1).getCubicRoots();
		var y=resolveRoots[0];
		var discrim=c3*c3/4-c2+y;
		if (Math.abs(discrim)<=Polynomial.TOLERANCE)
			discrim=0;
		if (discrim>0) {
			var e=Math.sqrt(discrim);
			var t1=3*c3*c3/4-e*e-2*c2;
			var t2=(4*c3*c2-8*c1-c3*c3*c3)/(4*e);
			var plus=t1+t2;var minus=t1-t2;
			if (Math.abs(plus)<=Polynomial.TOLERANCE)
				plus=0;
			if (Math.abs(minus)<=Polynomial.TOLERANCE)
				minus=0;
			if (plus>=0) {
				var f=Math.sqrt(plus);
				results.push(-c3/4 + (e+f)/2);
				results.push(-c3/4 + (e-f)/2);
			}
			if (minus>=0) {
				var f=Math.sqrt(minus);
				results.push(-c3/4 + (f-e)/2);
				results.push(-c3/4 - (f+e)/2);
			}
		} else if (discrim<0) {
		} else {
			var t2=y*y-4*c0;
			if (t2>=-Polynomial.TOLERANCE) {
				if (t2<0)
					t2=0;
				t2=2*Math.sqrt(t2);
				t1=3*c3*c3/4-2*c2;
				if (t1+t2>=Polynomial.TOLERANCE) {
					var d=Math.sqrt(t1+t2);
					results.push(-c3/4 + d/2);
					results.push(-c3/4 - d/2);
				}
				if (t1-t2>=Polynomial.TOLERANCE) {
					var d=Math.sqrt(t1-t2);
					results.push(-c3/4 + d/2);
					results.push(-c3/4 - d/2);
				}
			}
		}
	}
	return results;
}





function Vector2D(x,y){if(arguments.length>0){this.init(x,y);}}
Vector2D.prototype.init=function(x,y){this.x=x;this.y=y;};
Vector2D.prototype.length=function(){return Math.sqrt(this.x*this.x+this.y*this.y);};
Vector2D.prototype.dot=function(that){return this.x*that.x+this.y*that.y;};
Vector2D.prototype.cross=function(that){return this.x*that.y-this.y*that.x;}
Vector2D.prototype.unit=function(){return this.divide(this.length());};
Vector2D.prototype.unitEquals=function(){this.divideEquals(this.length());return this;};
Vector2D.prototype.add=function(that){return new Vector2D(this.x+that.x,this.y+that.y);};
Vector2D.prototype.addEquals=function(that){this.x+=that.x;this.y+=that.y;return this;};
Vector2D.prototype.subtract=function(that){return new Vector2D(this.x-that.x,this.y-that.y);};
Vector2D.prototype.subtractEquals=function(that){this.x-=that.x;this.y-=that.y;return this;};
Vector2D.prototype.multiply=function(scalar){return new Vector2D(this.x*scalar,this.y*scalar);};
Vector2D.prototype.multiplyEquals=function(scalar){this.x*=scalar;this.y*=scalar;return this;};
Vector2D.prototype.divide=function(scalar){return new Vector2D(this.x/ scalar, this.y /scalar);};
Vector2D.prototype.divideEquals=function(scalar){this.x/=scalar;this.y/=scalar;return this;};
Vector2D.prototype.perp=function(){return new Vector2D(-this.y,this.x);};
Vector2D.fromPoints=function(p1,p2){return new Vector2D(p2.x-p1.x,p2.y-p1.y);};
