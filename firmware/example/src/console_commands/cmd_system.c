/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "cmd_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

static void register_free(void);
static void register_heap(void);
static void register_version(void);
static void register_restart(void);
#if WITH_TASKS_INFO
static void register_tasks(void);
static void register_stats(void);
static void register_interval_stats(void);
#endif

void register_system(void)
{
    register_free();
    register_heap();
    register_version();
    register_restart();
#if WITH_TASKS_INFO
    register_tasks();
    register_stats();
    register_interval_stats();
#endif
}

/* 'version' command */
int get_version(int argc, char **argv) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("IDF Version:%s\r\n", esp_get_idf_version());
    printf("Chip info:\r\n");
    printf("\tmodel:%s\r\n", chip_info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
    printf("\tcores:%d\r\n", chip_info.cores);
    uint32_t size_flash_chip;
+   esp_flash_get_size(NULL, &size_flash_chip);
    printf("\tfeature:%s%s%s%s%lu%s\r\n",
           chip_info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           chip_info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           chip_info.features & CHIP_FEATURE_BT ? "/BT" : "",
           chip_info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           size_flash_chip / (1024 * 1024), " MB");
    printf("\trevision number:%d\r\n", chip_info.revision);
    return 0;
}


static void register_version(void)
{
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "version",
        .help = "Get version of chip and SDK",
        .hint = NULL,
        .func = &get_version,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}

/** 'restart' command restarts the program */

int restart(int argc, char **argv) {
    ESP_LOGI(TAG_COMMAND_SYSTENM, "Restarting");
    esp_restart();
}

static void register_restart(void)
{
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "restart",
        .help = "Software reset of the chip",
        .hint = NULL,
        .func = &restart,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}

/** 'free' command prints available heap memory */

int free_mem(int argc, char **argv) {
    printf("%lu\n", esp_get_free_heap_size());
    return 0;
}

static void register_free(void)
{
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "free",
        .help = "Get the current size of free heap memory",
        .hint = NULL,
        .func = &free_mem,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}

/* 'heap' command prints minumum heap size */
int heap_size(int argc, char **argv) {
    uint32_t heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    printf("min heap size: %lu\n", heap_size);
    return 0;
}

static void register_heap(void)
{
    const ch405_labs_esp_console_cmd_t heap_cmd = {
        .command = "minheap",
        .help = "Get minimum size of free heap memory that was available during program execution",
        .hint = NULL,
        .func = &heap_size,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&heap_cmd) );

}

/** 'tasks' command prints the list of tasks and related information */
#if WITH_TASKS_INFO

int tasks_info(int argc, char **argv) {
    const size_t bytes_per_task = 40; /* see vTaskList description */
    char *task_list_buffer = malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == NULL) {
        ESP_LOGE(TAG_COMMAND_SYSTENM, "failed to allocate buffer for vTaskList output");
        return 1;
    }
    fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
#ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
    fputs("\tAffinity", stdout);
#endif
    fputs("\n", stdout);
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    return 0;
}

static void register_tasks(void)
{
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "tasks",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &tasks_info,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}

int stats_info(int argc, char **argv) {

	char*	pBuffer;

	pBuffer = malloc(2048);
	if (pBuffer)
	{
		vTaskGetRunTimeStats(pBuffer);
		printf("TaskName\tCycles\t\tPercent\r\n");
		printf("===============================================\r\n");
		printf("%s", pBuffer);
		free(pBuffer);
	}
	return 0;


}

static void register_stats(void)
{
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "taskstats",
        .help = "Get stats about running tasks",
        .hint = NULL,
        .func = &stats_info,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}

static esp_err_t print_real_time_stats(TickType_t xTicksToWait) {
    TaskStatus_t *start_array = NULL, *end_array = NULL;
    UBaseType_t start_array_size, end_array_size;
    uint32_t start_run_time, end_run_time;
    esp_err_t ret;

    //Allocate array to store current task states
    start_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    start_array = malloc(sizeof(TaskStatus_t) * start_array_size);
    if (start_array == NULL) {
        ret = ESP_ERR_NO_MEM;
        goto exit;
    }
    //Get current task states
    start_array_size = uxTaskGetSystemState(start_array, start_array_size, &start_run_time);
    if (start_array_size == 0) {
        ret = ESP_ERR_INVALID_SIZE;
        goto exit;
    }

    vTaskDelay(xTicksToWait);

    //Allocate array to store tasks states post delay
    end_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    end_array = malloc(sizeof(TaskStatus_t) * end_array_size);
    if (end_array == NULL) {
        ret = ESP_ERR_NO_MEM;
        goto exit;
    }
    //Get post delay task states
    end_array_size = uxTaskGetSystemState(end_array, end_array_size, &end_run_time);
    if (end_array_size == 0) {
        ret = ESP_ERR_INVALID_SIZE;
        goto exit;
    }

    //Calculate total_elapsed_time in units of run time stats clock period.
    uint32_t total_elapsed_time = (end_run_time - start_run_time);
    if (total_elapsed_time == 0) {
        ret = ESP_ERR_INVALID_STATE;
        goto exit;
    }

    printf("| Task | Run Time | Percentage\n");
    //Match each task in start_array to those in the end_array
    for (int i = 0; i < start_array_size; i++) {
        int k = -1;
        for (int j = 0; j < end_array_size; j++) {
            if (start_array[i].xHandle == end_array[j].xHandle) {
                k = j;
                //Mark that task have been matched by overwriting their handles
                start_array[i].xHandle = NULL;
                end_array[j].xHandle = NULL;
                break;
            }
        }
        //Check if matching task found
        if (k >= 0) {
            uint32_t task_elapsed_time = end_array[k].ulRunTimeCounter - start_array[i].ulRunTimeCounter;
            uint32_t percentage_time = (task_elapsed_time * 100UL) / (total_elapsed_time * portNUM_PROCESSORS);
            printf("| %s | %lu | %lu%%\n", start_array[i].pcTaskName, task_elapsed_time, percentage_time);
        }
    }

    //Print unmatched tasks
    for (int i = 0; i < start_array_size; i++) {
        if (start_array[i].xHandle != NULL) {
            printf("| %s | Deleted\n", start_array[i].pcTaskName);
        }
    }
    for (int i = 0; i < end_array_size; i++) {
        if (end_array[i].xHandle != NULL) {
            printf("| %s | Created\n", end_array[i].pcTaskName);
        }
    }
    ret = ESP_OK;

exit:    //Common return path
    free(start_array);
    free(end_array);
    return ret;
}


int interval_stats_info(int argc, char **argv) {
	print_real_time_stats(configTICK_RATE_HZ); //Calcula las estadisticas durante 1 segundo
	return 0;
}

static void register_interval_stats(void)
{
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "intstats",
        .help = "Get stats about running tasks during interval",
        .hint = NULL,
        .func = &interval_stats_info,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}

#endif // WITH_TASKS_INFO

