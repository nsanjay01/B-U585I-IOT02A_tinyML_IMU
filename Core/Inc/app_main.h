#ifdef __cplusplus
extern "C" {
#endif

int app_main(void); // This function will be called from main.c
void AI_Init(void);
float Run_Inference(float* window_data);

#ifdef __cplusplus
}
#endif