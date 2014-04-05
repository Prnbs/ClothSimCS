struct Particle
{
	float3 pos, velocity, acceleration, lastPos;
	float mass;
	bool isConstrained;
	int id;
};

struct Spring
{
	float restLength, stiffness, damping;
	Particle p1, p2;
};

/*
void calculateSpringForce(in Spring _springs, out Spring _out)
{
	float restLength = _springs.restLength;
	float3 dist = _springs.p1.pos - _springs.p2.pos;
	float currentLength = length(dist);

	float extension = currentLength - restLength;

	float3 vel = _springs.p2.velocity - _springs.p1.velocity;

	float3 sNorm = _springs.p1.pos - _springs.p2.pos;
	float3 sNormResult;
	if(length(sNorm) !=0 )
	{
		sNormResult = normalize(sNorm);
	}

	float3 stiffnessForce = float3(0,0,0);
	float3 springForce = float3(0,0,0);
	float3 dampingForce = float3(0,0,0);

	stiffnessForce = 1.0f * _springs.stiffness * extension * sNormResult;
	dampingForce = -1.0f * _springs.damping * vel;

	springForce = stiffnessForce + dampingForce;

	_out.p1.acceleration += -springForce / _springs.p1.mass;
	_out.p2.acceleration += springForce / _springs.p2.mass;
}*/
/*
void accumulateForces(Particle _particles, float3 force) //no function needed
{
	_particles.acceleration = float3(0,0,0);
	_particles.acceleration += force / _particles.mass;
}*/

StructuredBuffer<Spring> Input : register( t0 );
RWStructuredBuffer<Spring> DataOut : register( u0 );

[numthreads(1, 1, 1)]
void CSMain( uint3 dTid : SV_DispatchThreadID )
{
	//calculateSpringForce(Input[dTid], DataOut[dTid]);
	Spring _springs = Input[dTid.x];
	Spring _out = DataOut[dTid.x];
	
	float restLength = _springs.restLength;
	float3 dist = _springs.p1.pos - _springs.p2.pos;
	float currentLength = length(dist);

	float extension = currentLength - restLength;

	float3 vel = _springs.p2.velocity - _springs.p1.velocity;

	float3 sNorm = _springs.p1.pos - _springs.p2.pos;
	float3 sNormResult;
	if(length(sNorm) !=0 )
	{
		sNormResult = normalize(sNorm);
	}

	float3 stiffnessForce = float3(0,0,0);
	float3 springForce = float3(0,0,0);
	float3 dampingForce = float3(0,0,0);

	stiffnessForce = 1.0f * _springs.stiffness * extension * sNormResult;
	dampingForce = -1.0f * _springs.damping * vel;

	springForce = stiffnessForce + dampingForce;

	_out.p1.acceleration += -springForce / _springs.p1.mass;
	_out.p2.acceleration += springForce / _springs.p2.mass;
	
	
	accumulateForces(DataOut[dTid], float3(0,1,0));
}


/*verlet(Particle[] _particles, float _timeStep)
{
	float radius = 2.0f;
	float3 spherePos = float3(0.0f,-4.0f, 4.0f);

	for(unsigned int i = 0; i < _particles.size(); i++)
    {
        if(!_particles[i].isConstrained)
        {
            pjs::float3 previousPos = _particles[i].pos;
            _particles[i].pos = _particles[i].pos + (_particles[i].pos - _particles[i].lastPos)
                    + ( _particles[i].acceleration * _timeStep) * _timeStep;
            _particles[i].lastPos = previousPos;

        }
    }
}*/