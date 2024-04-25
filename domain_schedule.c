#include <util.h>
#include <object/structures.h>
#include <model/statedata.h>

/* TODO: Can we somehow incorporate this as a configuration of sel4cp SDF?
 * Currently it's not possible because the kernel needs this. We COULD pass it in
 * from the loader to the kernel during boot, but the loader is built with the SDK. Same problem.
 */
/* Each length unit is 1 millisecond. */
const dschedule_t ksDomSchedule[] = {
    { .domain = 0, .length = 3 }, 
    { .domain = 1, .length = 1 },
    { .domain = 2, .length = 3 },
};

const word_t ksDomScheduleLength = ARRAY_SIZE(ksDomSchedule);

