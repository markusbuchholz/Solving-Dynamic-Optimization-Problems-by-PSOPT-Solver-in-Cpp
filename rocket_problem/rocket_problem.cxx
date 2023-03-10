//////////////////////////////////////////////////////////////////////////
//////////////////           rocket_problem.cxx      /////////////////////
//////////////////////////////////////////////////////////////////////////


#include "psopt.h"

//////////////////////////////////////////////////////////////////////////
///////////////////  Define the end point (Mayer) cost function //////////
//////////////////////////////////////////////////////////////////////////

adouble endpoint_cost(adouble *initial_states, adouble *final_states,
                      adouble *parameters, adouble &t0, adouble &tf,
                      adouble *xad, int iphase, Workspace *workspace)
{
 
    return tf;
}

//////////////////////////////////////////////////////////////////////////
///////////////////  Define the integrand (Lagrange) cost function  //////
//////////////////////////////////////////////////////////////////////////

adouble integrand_cost(adouble *states, adouble *controls, adouble *parameters,
                       adouble &time, adouble *xad, int iphase, Workspace *workspace)
{

    return 0.0;
}

//////////////////////////////////////////////////////////////////////////
///////////////////  Define the DAE's ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void dae(adouble *derivatives, adouble *path, adouble *states,
         adouble *controls, adouble *parameters, adouble &time,
         adouble *xad, int iphase, Workspace *workspace)
{
    adouble altitude_dot, speed_dot, mass_dot;

    adouble altitude = states[0];
    adouble speed = states[1];
    adouble mass = states[2];

    adouble thrust = controls[0];

    double exhaust_velocity = 2.349;
    double gravity = 1.0;

    altitude_dot = speed;
    speed_dot = (thrust - 0.2 * speed * speed) / mass; //-gravity + thrust/mass;
    mass_dot = -0.01 * thrust * thrust;                //-thrust/exhaust_velocity;

    derivatives[0] = altitude_dot;
    derivatives[1] = speed_dot;
    derivatives[2] = mass_dot;
}

////////////////////////////////////////////////////////////////////////////
///////////////////  Define the events function ////////////////////////////
////////////////////////////////////////////////////////////////////////////

void events(adouble *e, adouble *initial_states, adouble *final_states,
            adouble *parameters, adouble &t0, adouble &tf, adouble *xad,
            int iphase, Workspace *workspace)
{
    //init
    adouble altitude_i = initial_states[0];
    adouble speed_i = initial_states[1];
    adouble mass_i = initial_states[2];
    //final
    adouble altitude_f = final_states[0];
    adouble speed_f = final_states[1];

    e[0] = altitude_i;
    e[1] = speed_i;
    e[2] = mass_i;
    e[3] = altitude_f;
    e[4] = speed_f;
}

///////////////////////////////////////////////////////////////////////////
///////////////////  Define the phase linkages function ///////////////////
///////////////////////////////////////////////////////////////////////////

void linkages(adouble *linkages, adouble *xad, Workspace *workspace)
{
    // No linkages as this is a single phase problem
}

////////////////////////////////////////////////////////////////////////////
///////////////////  Define the main routine ///////////////////////////////
////////////////////////////////////////////////////////////////////////////

int main(void)
{
    ////////////////////////////////////////////////////////////////////////////
    ///////////////////  Declare key structures ////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    Alg algorithm;
    Sol solution;
    Prob problem;

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////  Register problem name  ////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    problem.name = "Rocket Problem";
    problem.outfilename = "rocket_problem.txt";

    ////////////////////////////////////////////////////////////////////////////
    ////////////  Define problem level constants & do level 1 setup ////////////
    ////////////////////////////////////////////////////////////////////////////

    problem.nphases = 1;
    problem.nlinkages = 0;

    psopt_level1_setup(problem);

    /////////////////////////////////////////////////////////////////////////////
    /////////   Define phase related information & do level 2 setup  ////////////
    /////////////////////////////////////////////////////////////////////////////

    problem.phases(1).nstates = 3;
    problem.phases(1).ncontrols = 1;
    problem.phases(1).nevents = 5;
    problem.phases(1).npath = 0;
    problem.phases(1).nodes << 50;

    psopt_level2_setup(problem, algorithm);

    int nodes = 70;

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////  Declare DMatrix objects to store results //////////////
    ////////////////////////////////////////////////////////////////////////////

    DMatrix x, u, t;
    DMatrix lambda, H;

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////  Enter problem bounds information //////////////////////
    ////////////////////////////////////////////////////////////////////////////

    double altitudeL = 0.0;
    double speedL = 0.0;
    double massL = 0.2;

    double altitudeU = 10.0;
    double speedU = 1.7;
    double massU = 1.0;

    double thrustL = -1.1;
    double thrustU = 1.1;

    double altitude_i = 0.0;
    double speed_i = 0.0;
    double mass_i = 1.0;

    double altitude_f = 10.0;
    double speed_f = 0.0;

    problem.phases(1).bounds.lower.states(0) = altitudeL;
    problem.phases(1).bounds.lower.states(1) = speedL;
    problem.phases(1).bounds.lower.states(2) = massL;

    problem.phases(1).bounds.upper.states(0) = altitudeU;
    problem.phases(1).bounds.upper.states(1) = speedU;
    problem.phases(1).bounds.upper.states(2) = massU;

    problem.phases(1).bounds.lower.controls(0) = thrustL;
    problem.phases(1).bounds.upper.controls(0) = thrustU;

    problem.phases(1).bounds.lower.events(0) = altitude_i;
    problem.phases(1).bounds.lower.events(1) = speed_i;
    problem.phases(1).bounds.lower.events(2) = mass_i;
    problem.phases(1).bounds.lower.events(3) = altitude_f;
    problem.phases(1).bounds.lower.events(4) = speed_f;

    problem.phases(1).bounds.upper.events = problem.phases(1).bounds.lower.events;

    problem.phases(1).bounds.lower.StartTime = 0.0;
    problem.phases(1).bounds.upper.StartTime = 0.0;

    problem.phases(1).bounds.lower.EndTime = 0.0;
    problem.phases(1).bounds.upper.EndTime = 100.0;

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////  Register problem functions  ///////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    problem.integrand_cost = &integrand_cost;
    problem.endpoint_cost = &endpoint_cost;
    problem.dae = &dae;
    problem.events = &events;
    problem.linkages = &linkages;

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////  Define & register initial guess ///////////////////////
    ////////////////////////////////////////////////////////////////////////////

    DMatrix states_guess(3, nodes);

    states_guess << linspace(altitude_i, altitude_f, nodes),
        linspace(speed_i, speed_f, nodes),
        linspace(mass_i, massL, nodes);

    problem.phases(1).guess.controls = 0.5 * (thrustL + thrustU) * ones(1, nodes);
    problem.phases(1).guess.states = states_guess;
    problem.phases(1).guess.time = linspace(0.0, 1.5, nodes);

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////  Enter algorithm options  //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    algorithm.nlp_iter_max = 1000;
    algorithm.nlp_tolerance = 1.e-6;
    algorithm.nlp_method = "IPOPT";
    algorithm.scaling = "automatic";
    algorithm.derivatives = "automatic";

    ////////////////////////////////////////////////////////////////////////////
    ///////////////////  Now call PSOPT to solve the problem   //////////////////
    ////////////////////////////////////////////////////////////////////////////

    psopt(solution, problem, algorithm);

    ////////////////////////////////////////////////////////////////////////////
    ///////////  Extract relevant variables from solution structure   //////////
    ////////////////////////////////////////////////////////////////////////////

    x = solution.get_states_in_phase(1);
    u = solution.get_controls_in_phase(1);
    t = solution.get_time_in_phase(1);
   // lambda = solution.get_dual_costates_in_phase(1);
   // H = solution.get_dual_hamiltonian_in_phase(1);

    ////////////////////////////////////////////////////////////////////////////
    ///////////  Save solution data to files if desired ////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    Save(x, "x.dat");
    Save(u, "u.dat");
    Save(t, "t.dat");
   // Save(lambda, "lambda.dat");
   // Save(H, "H.dat");

    ////////////////////////////////////////////////////////////////////////////
    ///////////  Plot some results if desired (requires gnuplot) ///////////////
    ////////////////////////////////////////////////////////////////////////////

    plot(t, x, problem.name, "time (s)", "states", "altitude speed mass");

    plot(t, u, problem.name, "time (s)", "control", "thrust");

    plot(t, x, problem.name, "time (s)", "states", "altitude speed mass",
         "pdf", "rocket_states.pdf");

    plot(t, u, problem.name, "time (s)", "control", "thrust",
         "pdf", "rocket_control.pdf");

}

////////////////////////////////////////////////////////////////////////////
///////////////////////      END OF FILE     ///////////////////////////////
////////////////////////////////////////////////////////////////////////////
