#include "BehaviorsModule.h"
#include "PyConstants.h"

#include <iostream>

static const unsigned int NUM_PYTHON_RESTARTS_MAX = 3;

namespace man {
	namespace behaviors {

		BehaviorsModule::BehaviorsModule()//unsigned int player_num, unsigned int team_num)
			: portals::Module(),
			  brain_module(NULL),
			  brain_instance(NULL),
			  num_crashed(0),
			  player_number(1),//player_num),
			  team_number(27),//team_num),
			  in_proto(),
			  in_size(),
			  out_serial(),
			  out_proto(),
			  out_size_t(),
			  out_size(),
			  ledCommandOut(base())
		{
			// Build Python list of correct length
			// this cast might totally fail... but if it doesn't,
			//  this is cleaner
			in_list_serials = PyList_New((Py_ssize_t)NUM_IN_MESSAGES);

			// Initialize python and brain.
			initializePython();
			getBrainInstance();
		}

		void BehaviorsModule::run_()
		{
			// If in error, try restarting automatically.
			// If too many consecutive failures, stop python and stop trying.

			if (num_crashed > NUM_PYTHON_RESTARTS_MAX) {
				return;
			} else if (num_crashed == NUM_PYTHON_RESTARTS_MAX) {
				Py_Finalize();
				num_crashed++;
				return;
			} else if (error_state && num_crashed < NUM_PYTHON_RESTARTS_MAX) {
				this->reload_hard();
				error_state = false;
				num_crashed++;
			}

			// Profiler
			//PROF_ENTER(P_PYTHON);

			//PROF_ENTER(P_PYTHON);

			// Call main run() method of Brain
			if (brain_instance != NULL) {
				// Serialize messages to send into python.
				serializeInMessages();
				// Assert: in_list_serials is properly constructed.

				// Calls the run method with no args.
				PyObject *result = PyObject_CallMethodObjArgs(brain_instance,
															  Py_BuildValue(py_string_format,"run",3),
															  in_list_serials,
															  NULL);
				if (result == NULL) {
					// set Behaviors in error state
					error_state = true;
					// report error
					fprintf(stderr, "Error occurred in behaviors.Brain.run() method\n");
					if (PyErr_Occurred()) {
						PyErr_Print();
					} else {
						fprintf(stderr, "  No Python exception information available\n");
					}
				} else {
					// Retrieve serialized out messages.
					parseOutMessages(result);
					// Send out messages.
					portals::Message<messages::LedCommand> ledCommand(0);
					ledCommand.get()->ParseFromArray(out_proto[0],out_size[0]);
					ledCommandOut.setMessage(ledCommand);

					Py_DECREF(result);
				}
			}
			// Profiler
			//PROF_EXIT(P_PYRUN);

			//PROF_EXIT(P_PYTHON);
		}

		void BehaviorsModule::serializeInMessages()
		{
			gameStateIn.latch();
			// Size that serialized message will be.
			in_size[GAME_STATE_IN] = gameStateIn.message().ByteSize();
			// Set in_proto to be the serialized message.
			gameStateIn.message().SerializeToArray(in_proto[GAME_STATE_IN],
												   in_size[GAME_STATE_IN]);
			in_serial[GAME_STATE_IN] = Py_BuildValue(py_string_format,
													 in_proto[GAME_STATE_IN],
													 in_size[GAME_STATE_IN]);

			filteredBallIn.latch();
			in_size[FILTERED_BALL_IN] = filteredBallIn.message().ByteSize();
			filteredBallIn.message().SerializeToArray(in_proto[FILTERED_BALL_IN],
													  in_size[FILTERED_BALL_IN]);
			in_serial[FILTERED_BALL_IN] = Py_BuildValue(py_string_format,
														in_proto[FILTERED_BALL_IN],
														in_size[FILTERED_BALL_IN]);

			for (int i=0; i<NUM_PLAYERS_PER_TEAM; i++) {
				worldModelIn[i].latch();
				in_size[WORLD_MODEL_IN+i] = worldModelIn[i].message().ByteSize();
				worldModelIn[i].message().SerializeToArray(in_proto[WORLD_MODEL_IN+i],
														in_size[WORLD_MODEL_IN+i]);
				in_serial[WORLD_MODEL_IN+i] = Py_BuildValue(py_string_format,
															in_proto[WORLD_MODEL_IN+i],
															in_size[WORLD_MODEL_IN+i]);
			}

			// in_serial[] and in_size[] are properly constructed
			for (Py_ssize_t i=0; i<NUM_IN_MESSAGES; i++) {
				PyList_SetItem(in_list_serials, i, in_serial[i]);
			}
		}

		void BehaviorsModule::parseOutMessages(PyObject *tuple)
		{
			PyArg_UnpackTuple(tuple, "name", NUM_OUT_MESSAGES, NUM_OUT_MESSAGES, &out_serial[0], &out_serial[1]);
			for (int i=0; i<NUM_OUT_MESSAGES; i++) {
				PyString_AsStringAndSize(out_serial[i], &out_proto[i], out_size_t[i]);
				out_size[i] = PyLong_AsLong(PyLong_FromSsize_t(*out_size_t[i]));
			}
		}

		void BehaviorsModule::reload_hard()
		{
			printf("Reloading Behaviors Python interpreter\n");
			// finalize and reinitialize the Python interpreter
			Py_Finalize();
			// load C extension modules
			initializePython();
			// instantiate a Brain instance
			getBrainInstance();
		}

		void BehaviorsModule::initializePython()
		{
# ifdef DEBUG_BEHAVIORS_INITIALIZATION
			printf("  Initializing interpreter and extension modules\n");
# endif

			Py_Initialize();
			modifySysPath();
			brain_module = NULL;

			// These boosted files are currently still necessary.
			// Initialize low-level modules
			c_init_noggin_constants();
			c_init_objects();
			c_init_goalie();
		}

		void BehaviorsModule::modifySysPath()
		{
			//TODO: figure out if we still need this
			// Enter the current working directory into the pyton module path
#if defined OFFLINE || defined STRAIGHT
			string dir1 = NBITES_DIR"/build/tool";
			string dir2 = NBITES_DIR"/build/tool/man";
			const char* cwd = "";
#else
			const char* cwd = "/home/nao/nbites/lib";
#endif

#ifdef DEBUG_BEHAVIORS_INITIALIZATION
			printf("  Adding %s to sys.path\n", cwd);
#endif

			PyObject *sys_module = PyImport_ImportModule("sys");
			if (sys_module == NULL) {
				fprintf(stderr, "** Error importing sys module: **");
				if (PyErr_Occurred())
					PyErr_Print();
				else
					fprintf(stderr, "** No Python exception information available **");
			}else {
				PyObject *dict = PyModule_GetDict(sys_module);
				PyObject *path = PyDict_GetItemString(dict, "path");
#ifdef OFFLINE
				PyList_Append(path, PyString_FromString(dir1.c_str()));
				PyList_Append(path, PyString_FromString(dir2.c_str()));
#else
				PyList_Append(path, PyString_FromString(cwd));
#endif
				Py_DECREF(sys_module);
			}
		}

		bool BehaviorsModule::import_brain()
		{
			// Load Brain module
			if (brain_module == NULL) {
				// Import brain module
#ifdef DEBUG_BEHAVIORS_INITIALIZATION
				printf("  Importing behaviors.Brain\n");
#endif
				brain_module = PyImport_ImportModule("python.behaviors.Brain");
			}

			if (brain_module == NULL) {
				// error, couldn't import behaviors.Brain
				fprintf(stderr, "Error importing behaviors.Brain module\n");
				if (PyErr_Occurred())
					PyErr_Print();
				else
					fprintf(stderr, "  No Python exception information available\n");
				return false;
			}

			return true;
		}

		void BehaviorsModule::getBrainInstance()
		{
			if (brain_module == NULL)
				if (!import_brain())
					return;

			// drop old reference
			Py_XDECREF(brain_instance);
			// grab instantiate and hold a reference to a new behaviors.Brain.Brain()
			PyObject *dict = PyModule_GetDict(brain_module);
			PyObject *brain_class = PyDict_GetItemString(dict, "Brain");
			if (brain_class != NULL) {
				// Constructs brain object with args
				PyObject *brain_constructor_args = Py_BuildValue("(ii)",player_number,team_number);
				brain_instance = PyObject_CallObject(brain_class, brain_constructor_args);
			} else {
				brain_instance = NULL;
				fprintf(stderr, "Error accessing behaviors.Brain.Brain\n");
				if (PyErr_Occurred())
					PyErr_Print();
				else
					fprintf(stderr, "  No error available\n");
			}

			error_state = (brain_instance == NULL);
		}
	}
}
