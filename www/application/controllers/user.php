<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class User extends CI_Controller {

   public function __construct()
   {
      parent::__construct();
      $this->load->spark('ion_auth/2.2.4');
   }

	public function index()
	{
      if( $this->ion_auth->logged_in() == true ){
         $data['page_title'] = 'File List';
         $data['loggedin'] = true;
         $data['user'] = $this->getUserInfo();
         $this->load->view('user/list', $data);
      }else{
         $data = array(
            'page_title' => 'Login',
            'loggedin' => false
         );
         $this->load->view('user/index', $data);
      }
	}

   public function login()
   {
      $this->load->library('form_validation');

      $username = $this->input->post('username');
      $password = $this->input->post('password');

      if( $this->ion_auth->login($username, $password) == true ){
         // Login success
         echo "Login successs";
      }else{
         // Login Failed
         echo "Login Failed. ID: $username";
      }

   }

   public function register()
   {
      $username = $this->input->post('username');
      $password = $this->input->post('password');
      $password2 = $this->input->post('password2');
      $email = $this->input->post('email');
      $profile = array();

      if( $this->ion_auth->email_check($email) == false ){
         // Email has not been used
         $result = $this->ion_auth->register($username, $password, $email, $profile, '');
         if( $result == true ){
            echo "New user: $username";
         }else{
            echo "Reg Failed";
         }
      }else{
         // Email has been used
         echo "This email has been regieterd.";
      }
   }

   public function checkID($id)
   {
      // code...
   }

   public function checkMail($mail)
   {
      // code...
   }

   private function getUserInfo($id="")
   {
      $info = $this->ion_auth->user()->row();
      $user =  array(
         'sn' => $info->id ,
         'id' => $info->username,
         'email' => $info->email
      );
      return $user;
   }
}
