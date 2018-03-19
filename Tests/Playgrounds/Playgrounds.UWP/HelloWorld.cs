using System;
using System.Threading.Tasks;
using Windows.Storage;
using Urho;
using Urho.Actions;
using Urho.Urho2D;
using BackgroundRenderer;

namespace Playgrounds.UWP
{
	public class HelloWorld : Application
	{
		MonoDebugHud debugHud;

		public HelloWorld(ApplicationOptions options = null) : base(options) { }

		protected override async void Start()
		{
			//Renderer.SetDefaultRenderPath(CoreAssets.RenderPaths.ForwardHWDepth);
			var scene = new Scene();
			scene.CreateComponent<Octree>();

		    var videoPlayer = new VideoPlayer();
            var screenMaterial = new Material();
            screenMaterial.SetTechnique(0, ResourceCache.GetTechnique("Techniques/DiffUnlit.xml"));
            screenMaterial.SetTexture(TextureUnit.Diffuse, videoPlayer.GetTexture());
            screenMaterial.DepthBias = new BiasParameters(-0.00001f, 0.0f);

            var videoNode = scene.CreateChild();
            videoNode.Position = new Vector3(0, 0, 2);
            videoNode.Rotation = new Quaternion(-90f, 0, 0);
            var videoModel = videoNode.CreateComponent<StaticModel>();
            videoModel.Model = CoreAssets.Models.Plane;
            videoModel.Material = screenMaterial;

            // Box
			var boxNode = scene.CreateChild();
			boxNode.Position = new Vector3(0, 0, 5);
			boxNode.Rotation = new Quaternion(60, 0, 30);
			boxNode.SetScale(1f);
			var modelObject = boxNode.CreateComponent<StaticModel>();
			modelObject.Model = ResourceCache.GetModel("Models/Box.mdl");
            //modelObject.SetMaterial(screenMaterial);

			boxNode.RunActions(new RepeatForever(new RotateBy(1,0, 90, 0)));
			//await Task.Delay(1000);
			//await ToMainThreadAsync();

			// Light
			var lightNode = scene.CreateChild(name: "light");
			lightNode.SetDirection(new Vector3(0.6f, -1.0f, 0.8f));
			lightNode.CreateComponent<Light>();

			// Camera
			var cameraNode = scene.CreateChild(name: "camera");
			var camera = cameraNode.CreateComponent<Camera>();

			// Viewport
			Renderer.SetViewport(0, new Viewport(scene, camera, null));

			// DebugHud
			debugHud = new MonoDebugHud(this);
			debugHud.Show();

			Input.TouchMove += Input_TouchMove;
			Input.TouchBegin += InputOnTouchBegin;
			Input.MouseMoved += Input_MouseMove;
			Input.MouseButtonDown += InputOnMouseButtonDown;
		}

		void InputOnTouchBegin(TouchBeginEventArgs touchBeginEventArgs)
		{
		}

		void InputOnMouseButtonDown(MouseButtonDownEventArgs mouseButtonDownEventArgs)
		{
		}

		void Input_MouseMove(MouseMovedEventArgs e)
		{
			debugHud.AdditionalText = $"Mouse: {e.X};{e.Y}";
		}

		void Input_TouchMove(TouchMoveEventArgs e)
		{
			debugHud.AdditionalText = $"Touch: {e.X};{e.Y}";
		}
	}

    public class VideoPlayer : Component
    {
        private Texture2D _texture;
        private BackgroundRenderer.Renderer _bg;

        public VideoPlayer()
        {
            var folder = ApplicationData.Current.LocalFolder;

            IntPtr dxDevice = Application.Current.Graphics.GetDevice();
            _bg = new BackgroundRenderer.Renderer((UInt64)dxDevice, 0);

            _texture = new Texture2D();
            _texture.SetSize(320, 180, Graphics.RGBFormat, TextureUsage.Rendertarget);
            _texture.FilterMode = TextureFilterMode.Bilinear;

            var surface = _texture.RenderSurface;
            Application.Renderer.RenderSurfaceUpdate += Renderer_RenderSurfaceUpdate;
        }

        private void Renderer_RenderSurfaceUpdate(RenderSurfaceUpdateEventArgs obj)
        {
            var surface = _texture.RenderSurface;

            if (surface == null || !surface.UpdateQueued) return;

            // Get the GPU Object to send to the texture.
            var gpuObject = _texture.GetGPUObject();

            _bg.Update((UInt64)gpuObject);

            surface.ResetUpdateQueued();
        }

        public void SetSource(string source)
        {
            //_bg.SetSource(source);
        }

        public Texture2D GetTexture()
        {
            return _texture;
        }
    }
        
}
